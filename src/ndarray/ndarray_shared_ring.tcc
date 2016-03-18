#include <algorithm>

namespace mf {

template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::initialize() {
	std::lock_guard<std::mutex> lock(positions_mutex_);
	reader_state_ = idle;
	writer_state_ = idle;
	end_time_ = -1;
	read_reaches_eof_ = false;
	base::initialize();
}


template<std::size_t Dim, typename T>
auto ndarray_shared_ring<Dim, T>::begin_write(std::size_t duration) -> section_view_type {
	// duration test, before waiting
	if(duration > base::total_duration()) throw std::invalid_argument("write duration larger than ring capacity");

	// fail if already writing (same or different thread)
	if(writer_state_ != idle) throw sequencing_error("already writing");
	
	// fail if eof already marked
	if(end_time_ != -1) throw sequencing_error("writer already marked eof");

	// mutex used to protect positions (read/write position), and to wait on condition variable
	std::unique_lock<std::mutex> lock(positions_mutex_);

	// fail if reader is waiting, and writer was also now going to wait
	if(base::writable_duration() < duration && reader_state_ == waiting)
		throw sequencing_error("deadlock detected: ring buffer reader was already waiting");

	// wait on it, until writable duration is sufficient
	// each time frame(s) are read/skipped writable_cv_ gets notified
	// robust against spurious wake-ups, does not start waiting if enough frames were already writable
	while(base::writable_duration() < duration) {
		// wait until new writable frames become available
		writer_state_ = waiting;
		writable_cv_.wait(lock);
		
		// reader cannot be waiting now, as begin_read() also detects deadlock
		assert(reader_state_ != waiting);
	}

	// now return view to writable frames
	// base::begin_write() not thread-safe, but positions_mutex_ is still locked until return 
	writer_state_ = accessing;
	return base::begin_write(duration);
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::end_write(std::size_t written_duration) {
	end_write(written_duration, false);
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::end_write(std::size_t written_duration, bool eof) {
	// fail if not writing (begin_write() not called before)
	if(writer_state_ == idle) throw sequencing_error("was not writing");
	
	{
		std::lock_guard<std::mutex> lock(positions_mutex_);

		// set end time if eof marked: prevents any future writes
		if(eof) end_time_ = base::writable_time_span().start_time() + written_duration;

		// update positions in base class
		base::end_write(written_duration);
	}
	
	writer_state_ = idle;
	
	// notify condition variable: more readable frames have become available
	readable_cv_.notify_one();
}


template<std::size_t Dim, typename T>
auto ndarray_shared_ring<Dim, T>::begin_read(std::size_t duration) -> section_view_type {
	if(duration > base::total_duration()) throw std::invalid_argument("read duration larger than ring capacity");
	if(reader_state_ != idle) throw sequencing_error("already reading");

	read_reaches_eof_ = false;

	std::unique_lock<std::mutex> lock(positions_mutex_);

	time_unit requested_start_time = base::readable_time_span().start_time();
	if(end_time_.load() <= requested_start_time + duration) {
		duration = end_time_.load() - requested_start_time;
		read_reaches_eof_ = true;
	}

	if(base::readable_duration() < duration && writer_state_ == waiting)
		throw sequencing_error("deadlock detected: ring buffer writer was already waiting");

	while(base::readable_duration() < duration) {
		reader_state_ = waiting;
		readable_cv_.wait(lock);
		assert(writer_state_ != waiting);
		
		if(end_time_.load() <= requested_start_time + duration) {
			duration = end_time_.load() - requested_start_time;
			read_reaches_eof_ = true;
		}
	}
	
	reader_state_ = accessing;
	return base::begin_read(duration);
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::end_read(std::size_t read_duration) {
	if(reader_state_ == idle) throw sequencing_error("was not reading");
	{
		std::lock_guard<std::mutex> lock(positions_mutex_);
		base::end_read(read_duration);
	}
	reader_state_ = idle;
	writable_cv_.notify_one();
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::skip_available_(std::size_t duration) {
	// can only skip frames that are already readable
	assert(duration <= this->readable_duration());
	{
		// while mutex locked, update positions in base class
		std::lock_guard<std::mutex> lock(positions_mutex_);
		base::skip(duration);
	}
	
	// notify condition variable: more writable frames have become available
	writable_cv_.notify_one();
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::skip(std::size_t skip_duration) {
	// duration may be larger than ring capacity
	
	const auto initial_readable_duration = this->readable_duration();
	const auto total_duration = base::total_duration();
	
	if(skip_duration <= initial_readable_duration) {
		// simple case: only skip some of the already readable frames
		skip_available_(skip_duration);

	} else {
		auto remaining_skip_duration = skip_duration;
			
		// wait for whole buffer to fill up and then skip all frames
		// until less than buffer size remains to be skipped
		while(remaining_skip_duration >= total_duration) {
			auto readable_view = begin_read(total_duration); // ...wait for buffer to fill up entirely
			if(readable_view.shape().front() == total_duration) {
				// skip all and notify writable_cv_
				skip_available_(total_duration);
				remaining_skip_duration -= total_duration;
				reader_state_ = idle;
			} else {
				// writer end was marked, don't try to skip beyond
				skip_available_(readable_view.shape().front());
				remaining_skip_duration = 0;
				reader_state_ = idle;
				return;
			}
		}
		
		// now skip remaining frames, waiting for them if necessary
		begin_read(remaining_skip_duration);
		skip_available_(remaining_skip_duration);
		reader_state_ = idle;
		
		// there may be new readable frames already available (after skipped frames),
		// if >1 frames got written at a time
	}
}


template<std::size_t Dim, typename T>
time_unit ndarray_shared_ring<Dim, T>::shared_readable_duration() const {
	return shared_readable_time_span().duration();
}


template<std::size_t Dim, typename T>
time_span ndarray_shared_ring<Dim, T>::shared_readable_time_span() const {
	std::lock_guard<std::mutex> lock(positions_mutex_);
	time_span readable = base::readable_time_span();
	assert(end_time_ == -1 || readable.end_time() == end_time_);	
	return readable;
}


template<std::size_t Dim, typename T>
time_unit ndarray_shared_ring<Dim, T>::shared_writable_duration() const {
	return shared_writable_time_span().duration();
}


template<std::size_t Dim, typename T>
time_span ndarray_shared_ring<Dim, T>::shared_writable_time_span() const {
	std::lock_guard<std::mutex> lock(positions_mutex_);
	time_span writable = base::writable_time_span();
	if(end_time_ == -1) return writable;
	else return time_span(writable.start_time(), std::min(writable.end_time(), end_time_.load()));
}


template<std::size_t Dim, typename T>
bool ndarray_shared_ring<Dim, T>::eof_was_marked() const {
	return (end_time_ != -1);
}

}
