#include "shared_ring.h"
#include <algorithm>

namespace mf {

shared_ring::shared_ring
(const frame_array_properties& prop, bool seekable, time_unit end_time) :
	ring_(prop), seekable_(seekable), end_time_(end_time)
{
	if(seekable && end_time == -1) throw std::invalid_argument("end time must be defined when seekable");
}


void shared_ring::initialize() {
	std::lock_guard<std::mutex> lock(mutex_);
	ring_.initialize();
	reader_state_ = idle;
	writer_state_ = idle;
	read_start_time_ = 0;
}


auto shared_ring::begin_write(time_unit original_duration) -> section_view_type {	
	if(original_duration > capacity()) throw std::invalid_argument("write duration larger than ring capacity");
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	std::unique_lock<std::mutex> lock(mutex_);

	// write start position
	// if writer needs to wait (unlocking mutex), then reader may seek to another position
	// and this will change...
	time_unit write_start = ring_.write_start_time();

	// truncate write span if near end
	// if end not defined: duration will never cross end:
	// the buffer is not seekable so writes are sequential, and writer is responsible to mark end,
	// by setting mark_end in this function
	time_unit duration = original_duration;
	if(end_time_ != -1) {
		if(write_start + duration > end_time_) duration = end_time_ - write_start;
	}

	// if duration zero (possibly because at end), return zero-length view
	if(duration == 0) return ring_.begin_write(0);

	// prevent deadlock
	// c.f. begin_read_span
	if(ring_.writable_duration() < duration && ring_.readable_duration() < reader_state_)
		throw sequencing_error("deadlock detected: ring buffer reader was already waiting");


	while(ring_.writable_duration() < duration) {
		// wait for more frames to become available
		writer_state_ = duration;
		
		lock.unlock();
		// reader may send notification before wait starts: it gets stored in event
		event::wait_any(reader_idle_event_, reader_seek_event_);
		lock.lock();
		
		// received event: 1+ frame was written, or reader seeked
		// or neither (was notified when not waiting)
		// will recheck if enough frames are now available.

		// reader may now have seeked to other time
		if(ring_.write_start_time() != write_start) {
			MF_ASSERT(seekable_); // this cannot happen when buffer is not seekable
			
			// then retry with the new write position
			// only seek() changes write position (on reader thread), so no risk of repeated recursion
			
			writer_state_ = idle;
			lock.unlock(); // brief unlock: reader might seek again, or start waiting
			return begin_write(original_duration);
		}
	}

	// mutex_ is still locked
	writer_state_ = accessing;
	return ring_.begin_write(duration);
}


auto shared_ring::try_begin_write(time_unit original_duration) -> section_view_type {
	if(original_duration > capacity()) throw std::invalid_argument("write duration larger than ring capacity");
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	std::unique_lock<std::mutex> lock(mutex_);

	time_unit write_start = ring_.write_start_time();

	time_unit duration = original_duration;
	if(end_time_ != -1)
		if(write_start + duration > end_time_) duration = end_time_ - write_start;

	if(ring_.writable_duration() < duration) return section_view_type::null();

	section_view_type vw = ring_.begin_write(duration);
	writer_state_ = accessing;
	return vw;
}


bool shared_ring::wait_writable(event& break_event) {
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	{
		std::lock_guard<std::mutex> lock(mutex_);
		if(ring_.writable_duration() == 0 && ring_.readable_duration() < reader_state_)
			throw sequencing_error("deadlock detected: ring buffer reader was already waiting");
	}

	while(writable_time_span().duration() == 0) {
		event& ev = event::wait_any(reader_idle_event_, reader_seek_event_, break_event);
		if(ev == break_event) return false;
	}
	
	return true;
}


void shared_ring::end_write(time_unit written_duration, bool mark_end) {
	if(writer_state_ == idle) throw sequencing_error("was not writing");
	if(end_time_ != -1 && mark_end) throw std::invalid_argument("end already marked (or buffer is seekable)");	
	if(written_duration == 0 && mark_end) throw std::invalid_argument("cannot mark end when no frame was written");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		ring_.end_write(written_duration);
		
		if(mark_end) end_time_ = ring_.write_start_time();
	}
	
	writer_state_ = idle;
	writer_idle_event_.notify();
	
	// notify even if no frame was written:
	// seek() waits for writer to finish
}


auto shared_ring::begin_read_span(time_span span) -> section_view_type {
	if(span.duration() > capacity()) throw std::invalid_argument("read duration larger than ring capacity");
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// if span does not start immediatly at first readable frame, skip frames, or seek if possible
	if(span.start_time() > read_start_time_) {
		skip(span.start_time() - read_start_time_);	
	} else if(span.start_time() < read_start_time_) {
		if(seekable_) seek(span.start_time());
		else throw sequencing_error("read span cannot be in past for non-seekable ring");
	}

	// read_start_time_ is only affected by this reader thread
	assert(read_start_time_ == span.start_time());

	return begin_read(span.duration());
}


auto shared_ring::begin_read(time_unit original_duration) -> section_view_type {
	if(original_duration > capacity()) throw std::invalid_argument("read duration larger than ring capacity");
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// lock mutex for buffer state
	std::unique_lock<std::mutex> lock(mutex_);

	// truncate read duration if near end
	// if end time not defined: buffer is not seekable, and so end can only by marked by writer (end_write).
	// then if span is not readable yet, will need to recheck after waiting for additional frames
	time_unit duration = original_duration;
	if(end_time_ != -1)
		if(read_start_time_ + duration > end_time_) duration = end_time_ - read_start_time_;

	// if duration zero (possibly because at end), return zero view
	if(duration == 0) return ring_.begin_read(0);
	
	// prevent deadlock
	// writer_state_ may still be waiting, even after frames have become writable by end_write(),
	// because begin_read_span() might get the lock earlier than the in-progress begin_write()
	if(ring_.writable_duration() < writer_state_ && ring_.readable_duration() < duration)
		throw sequencing_error("deadlock detected: ring buffer writer was already waiting");

	// wait until duration becomes readable
	while(ring_.readable_duration() < duration) {
		reader_state_ = duration;
		// wait until more frames written (mutex unlocked during wait, and relocked after)
				
		lock.unlock();
		writer_idle_event_.wait();
		lock.lock();
		
		// writer might have marked end while reader was waiting
		if(end_time_ != -1 && read_start_time_ + duration >= end_time_)
			duration = end_time_ - read_start_time_;
	}
	
	// writer cannot have called end_write(0, true) (mark end when no frame written)
	MF_ASSERT(duration != 0);

	reader_state_ = accessing;
	return ring_.begin_read(duration);
}


auto shared_ring::try_begin_read(time_unit original_duration) -> section_view_type {
	if(original_duration > capacity()) throw std::invalid_argument("read duration larger than ring capacity");
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// lock mutex for buffer state
	std::unique_lock<std::mutex> lock(mutex_);

	// truncate read duration if near end
	// if end time not defined: buffer is not seekable, and so end can only by marked by writer (end_write).
	// then if span is not readable yet, will need to recheck after waiting for additional frames
	time_unit duration = original_duration;
	if(end_time_ != -1)
		if(read_start_time_ + duration > end_time_) duration = end_time_ - read_start_time_;
	
	if(ring_.readable_duration() < duration) return section_view_type::null();
	
	section_view_type vw = ring_.begin_read(duration);
	reader_state_ = accessing;
	return vw;
}



bool shared_ring::wait_readable(event& break_event) {
	if(reader_state_ != idle) throw sequencing_error("read not idle");

	{
		std::lock_guard<std::mutex> lock(mutex_);
		if(ring_.writable_duration() < writer_state_ && ring_.readable_duration() == 0)
			throw sequencing_error("deadlock detected: ring buffer writer was already waiting");
	}

	while(readable_time_span().duration() == 0) {
		event& ev = event::wait_any(writer_idle_event_, break_event);
		if(ev == break_event) return false;
	}
	
	return true;	
}



void shared_ring::end_read(time_unit read_duration) {
	if(reader_state_ == idle) throw sequencing_error("was not reading");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		
		ring_.end_read(read_duration);
		read_start_time_ += read_duration;
		MF_ASSERT(read_start_time_ == ring_.read_start_time());	
	}
	reader_state_ = idle;
	reader_idle_event_.notify();
}


void shared_ring::skip(time_unit skip_duration) {
	MF_EXPECTS(skip_duration >= 0);
	if(seekable_) {
		// truncate to end time
		time_unit target_time = std::min(read_start_time_ + skip_duration, end_time_.load());
		seek(target_time);
	} else {
		read_and_discard_(skip_duration);
	}
}


void shared_ring::read_and_discard_(time_unit skip_duration) {	
	MF_EXPECTS(skip_duration >= 0);

	// duration may be larger than ring capacity
	
	MF_ASSERT(! seekable_); // specific for non-seekable buffer
	
	const auto initial_readable_duration = readable_time_span().duration(); // locks mutex_
	
	// mutex_ briefly unlocked, but readable duration will not decrease
	
	if(skip_duration <= initial_readable_duration) {
		// simple case: only skip some of the already readable frames
		skip_available_(skip_duration);

	} else {
		auto remaining_skip_duration = skip_duration;
			
		// wait for whole buffer to fill up and then skip all frames
		// until less than buffer size remains to be skipped
		while(remaining_skip_duration >= capacity()) {
			auto readable_view = begin_read(capacity()); // ...wait for buffer to fill up entirely
			if(end_time_ != -1) {
				// end was marked, don't try to skip beyond
				MF_ASSERT(readable_view.end_time() == end_time_);
				skip_available_(readable_view.duration());
				remaining_skip_duration = 0;
				reader_state_ = idle;
				return;
			} else {
				// skip all and notify writable_cv_
				skip_available_(capacity());
				remaining_skip_duration -= capacity();
				reader_state_ = idle;
			}
		}
		
		// now skip remaining frames, waiting for them if necessary
		begin_read(remaining_skip_duration);
		skip_available_(remaining_skip_duration);
		reader_state_ = idle;
		
		// there may be new readable frames already available (after skipped frames),
		// if >1 frames get written at a time
	}
}


void shared_ring::skip_available_(time_unit skip_duration) {
	MF_EXPECTS(skip_duration >= 0);
	{
		std::lock_guard<std::mutex> lock(mutex_);
		MF_ASSERT(skip_duration <= ring_.readable_duration()); // ...can only skip frames that are already readable
		ring_.skip(skip_duration);
		read_start_time_ += skip_duration;
		MF_ASSERT(read_start_time_ == ring_.read_start_time());
	}
	reader_idle_event_.notify();
}


bool shared_ring::can_seek(time_unit target_time) const {
	if(!seekable_ || end_time_ == -1) return false;
	else return (target_time <= end_time_);
}


void shared_ring::seek(time_unit t) {	
	if(! seekable_) throw std::logic_error("ring buffer is not seekable");
	if(end_time_ == -1) throw std::logic_error("cannot seek when end time is not defined");
	if(t > end_time_) throw std::invalid_argument("cannot seek beyond end");

	// no need to do anything if already at time t
	if(t == read_start_time_) return;

	// lock the mutex
	// writer will not start or end while locked, and readable time span will not change
	std::unique_lock<std::mutex> lock(mutex_);

	bool already_readable = ring_.readable_time_span().includes(t);
		
	if(!already_readable && writer_state_ == accessing) {
		// target tile span is not in buffer, and writer is currently accessing data:
		// need to let the writer finish writing first 
				
		// allow writer to write its frames
		// works because end_write sets writer_state_ before notifying readable_cv_
				
		while(writer_state_ == accessing) {			
			lock.unlock();
			writer_idle_event_.wait();
			lock.lock();
		}
		// mutex is now locked again
		// writer now idle, or waiting if it started another begin_write() in meantime
		
		// retest if target time is now in readable span
		already_readable = ring_.readable_time_span().includes(t);
	}
	
	
	if(already_readable) {
		// target time is already in buffer: simply skip to it ("short seek")
		time_unit skip_duration = t - read_start_time_;
		ring_.skip(skip_duration);
		read_start_time_ += skip_duration;
		MF_ASSERT(read_start_time_ == ring_.read_start_time());
						
	} else {
		// writer is necessarily idle or waiting now
		MF_ASSERT(writer_state_ != accessing);

		// perform seek on ring buffer now ("long seek")
		ring_.seek(t);
		read_start_time_ = t;
		MF_ASSERT(read_start_time_ == ring_.read_start_time());
	}

	// notify writer: seeked, and new writable frames
	// if it was idle, notification is ignored
	// if it was waiting, begin_write() notices that write position has changed
	lock.unlock();
	reader_seek_event_.notify();
}


time_unit shared_ring::current_time() const {
	return ring_.current_time();
}


time_unit shared_ring::write_start_time() const {
	// atomic, so no need to lock
	return ring_.write_start_time();
}


time_unit shared_ring::read_start_time() const {
	// use atomic member, instead of non-atomic ring_.read_start_time()
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.read_start_time();
	
	//return read_start_time_;
}


time_span shared_ring::writable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	time_span writable = ring_.writable_time_span();
	if(end_time_ == -1) return writable;
	else if(current_time() == end_time_) return time_span(end_time_, end_time_);
	else return time_span(writable.start_time(), std::min(writable.end_time(), end_time_.load()));
}


time_span shared_ring::readable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.readable_time_span();
}


bool shared_ring::writer_reached_end() const {
	if(end_time_ == -1) return false;
	else return (write_start_time() == end_time_);
}


bool shared_ring::reader_reached_end() const {
	if(end_time_ == -1) return false;
	else return (read_start_time() == end_time_);
}


}
