#include <thread>
#include <chrono>
using namespace std::literals;

namespace mf {

template<std::size_t Dim, typename T>
ndarray_seekable_shared_ring<Dim, T>::ndarray_seekable_shared_ring
(const ndsize<Dim>& frames_shape, std::size_t duration, time_unit end_t) :
	ring_(frames_shape, duration), end_time_(end_t) { }


template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::initialize() {
	std::lock_guard<std::mutex> lock(mutex_);
	ring_.initialize();
	reader_state_ = idle;
	writer_state_ = idle;
	read_start_time_ = 0;
}


template<std::size_t Dim, typename T>
auto ndarray_seekable_shared_ring<Dim, T>::begin_write(time_unit original_duration) -> section_view_type {	
	if(original_duration > capacity()) throw std::invalid_argument("write duration larger than ring capacity");
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	std::unique_lock<std::mutex> lock(mutex_);

	// write start position
	// if writer needs to wait (unlocking mutex), then reader may seek to another position
	// and this will change...
	time_unit write_start = ring_.write_start_time();

	// truncate write span if near end
	time_unit duration = original_duration;
	if(write_start + duration > end_time_) duration = end_time_ - write_start;

	// if duration zero (possibly because at end), return zero view
	if(duration == 0) return section_view_type(write_start);

	// prevent deadlock
	// c.f. begin_read_span
	if(ring_.writable_duration() < duration && ring_.readable_duration() < reader_state_)
		throw sequencing_error("deadlock detected: ring buffer reader was already waiting");


	while(ring_.writable_duration() < duration) {
		// wait for more frames to become available
		writer_state_ = duration;
		writable_cv_.wait(lock);
		//
		// mutex unlocked, waiting...
		//
		
		// woke up: either spurious wake up, or 1+ frame was written, or reader seeked.
		// will recheck if enough frames are now available.

		// reader may now have seeked to other time
		if(ring_.write_start_time() != write_start) {
			// then retry with the new write position
			// only seek() changes write position (on reader thread), so no risk of repeated recursion

			MF_DEBUG("writer: recursion, seek while waiting");
			
			writer_state_ = idle;
			lock.unlock(); // brief unlock: reader might seek again, or start waiting
			return begin_write(original_duration);
		}
	}

	// mutex_ is still locked
	writer_state_ = accessing;
	return ring_.begin_write(duration);
}


template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::end_write(time_unit written_duration) {
	if(writer_state_ == idle) throw sequencing_error("was not writing");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		ring_.end_write(written_duration);
	}
	
	writer_state_ = idle;
	readable_cv_.notify_one();	
}


template<std::size_t Dim, typename T>
auto ndarray_seekable_shared_ring<Dim, T>::begin_read_span(time_span span) -> section_view_type {
	if(span.duration() > capacity()) throw std::invalid_argument("read duration larger than ring capacity");
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// truncate write span if near end
	if(span.start_time() > end_time_) throw sequencing_error("read span starts after eof");
	if(span.end_time() > end_time_) span = time_span(span.start_time(), end_time_);

	// if duration zero (possibly because at end), return zero view
	if(span.duration() == 0) return section_view_type(span.start_time());

	// skip frames if span does not start immediatly at first readable frame
	// seeks to new write position
	if(span.start_time() > read_start_time_)
		skip(span.start_time() - read_start_time_);	

	// read_start_time_ is only affected by this reader threar
	assert(read_start_time_ == span.start_time());

	// lock mutex for buffer state
	std::unique_lock<std::mutex> lock(mutex_);
	
	// prevent deadlock
	// writer_state_ may still be waiting, even after frames have become writable by end_write(),
	// because begin_read_span() might get the lock earlier than the in-progress begin_write()
	if(ring_.writable_duration() < writer_state_ && ring_.readable_duration() < span.duration())
		throw sequencing_error("deadlock detected: ring buffer writer was already waiting");

	// wait until duration becomes readable
	while(ring_.readable_duration() < span.duration()) {
		reader_state_ = span.duration();
		// wait until more frames written (mutex unlocked during wait, and relocked after)
		readable_cv_.wait(lock);
		//
		// mutex unlocked, waiting...
		//
	}
	
	reader_state_ = accessing;

	return ring_.begin_read(span.duration());
}


template<std::size_t Dim, typename T>
auto ndarray_seekable_shared_ring<Dim, T>::begin_read(time_unit duration) -> section_view_type {
	// read_start_time_ only changed by reader, so no need to block
	return begin_read_span(time_span(read_start_time_, read_start_time_ + duration));
}


template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::end_read(time_unit read_duration) {
	if(reader_state_ == idle) throw sequencing_error("was not reading");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		ring_.end_read(read_duration);
		read_start_time_ += read_duration;
	}
	reader_state_ = idle;
	
	// notify condition variable: more writable frames have become available
	writable_cv_.notify_one();
}


template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::skip(time_unit skip_duration) {
	seek(read_start_time_ + skip_duration);
}


template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::seek(time_unit t) {	
	if(t > end_time_) throw std::invalid_argument("cannot seek beyond end");

	// lock the mutex
	// writer will not start or end while locked, and readable time span will not change
	std::unique_lock<std::mutex> lock(mutex_);

	bool already_readable = ring_.readable_time_span().includes(t);
	
	MF_DEBUG("reader: seek to ", t, ", already_readable=", already_readable);
	
	if(!already_readable && writer_state_ == accessing) {
		// target tile span is not in buffer, and writer is currently accessing data:
		// need to let the writer finish writing first 
				
		// allow writer to write its frames
		// works because end_write sets writer_state_ before notifying readable_cv_
		while(writer_state_ == accessing) {
			readable_cv_.wait(lock);
			//
			// mutex unlocked, waiting...
			//
		}
		// mutex is now locked again
		// writer now idle, or waiting if it started another begin_write() in meantime
		
		// retest if target time is now in readable span
		already_readable = ring_.readable_time_span().includes(t);
	}
	
	
	if(already_readable) {
		// target time is already in buffer: simply skip to it
		time_unit skip_duration = t - read_start_time_;
		ring_.skip(skip_duration);
		read_start_time_ += skip_duration;
						
	} else {
		// writer is necessarily idle or waiting now
		assert(writer_state_ != accessing);

		// perform seek on ring buffer now
		ring_.seek(t);
		read_start_time_ = t;
	}

	// notify writer: seeked, and new writable frames
	// if it was idle, notification is ignored
	// if it was waiting, begin_write() notices that write position has changed
	lock.unlock();
	writable_cv_.notify_one();
}


template<std::size_t Dim, typename T>
time_unit ndarray_seekable_shared_ring<Dim, T>::current_time() const {
	return ring_.current_time();
}


template<std::size_t Dim, typename T>
time_unit ndarray_seekable_shared_ring<Dim, T>::write_start_time() const {
	// atomic, so no need to lock
	return ring_.write_start_time();
}


template<std::size_t Dim, typename T>
time_unit ndarray_seekable_shared_ring<Dim, T>::read_start_time() const {
	// use atomic member, instead of non-atomic ring_.read_start_time()
	return read_start_time_;
}


template<std::size_t Dim, typename T>
time_span ndarray_seekable_shared_ring<Dim, T>::writable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	time_span writable = ring_.writable_time_span();
	if(end_time_ == -1) return writable;
	else if(current_time() == end_time_) return time_span(end_time_, end_time_);
	else return time_span(writable.start_time(), std::min(writable.end_time(), end_time_));
}


template<std::size_t Dim, typename T>
time_span ndarray_seekable_shared_ring<Dim, T>::readable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.readable_time_span();
}


#ifndef NDEBUG
template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::debug_print(std::ostream& str) const {
	str << "ndarray_seekable_shared_ring: \n";
	if(mutex_.try_lock()) {
		ring_.debug_print(str);
		str << "reader state="
		    << reader_state_
		    << ", writer state="
		    << writer_state_
		    << ", end_time=" << end_time_ << ", read_start_time=" << read_start_time_;
		mutex_.unlock();
	} else {
		str << "cannot lock mutex for debug_print" << std::endl;
	}
}
#endif


}
