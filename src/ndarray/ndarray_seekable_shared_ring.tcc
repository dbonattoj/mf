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
auto ndarray_seekable_shared_ring<Dim, T>::begin_write(time_unit duration) -> section_view_type {
	if(duration > total_duration()) throw std::invalid_argument("write duration larger than ring capacity");
	if(writer_state_ != idle) throw sequencing_error("already writing");
	if(eof_was_marked()) throw sequencing_error("writer already marked eof");

	std::unique_lock<std::mutex> lock(mutex_);

	// if would wait, but reader already waiting:
	// fail because deadlock would occur
	if(ring_.writable_duration() < duration && reader_state_ == waiting)
		throw sequencing_error("deadlock detected: ring buffer reader was already waiting");

	while(ring_.writable_duration() < duration) {
		writer_state_ = waiting;
		writable_cv_.wait(lock);
		// woke up: either spurious wake up or 1+ frame was written:
		// will recheck if enough frames are now available.
		// if seekable, reader may now have seeked to other time

		// no need to recheck if reader is waiting:
		// writer_state_ remained waiting so reader couldn't have started waiting
		assert(reader_state_ != waiting);
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
auto ndarray_seekable_shared_ring<Dim, T>::begin_read_span(time_span span, bool& reaches_eof) -> section_view_type {
	reaches_eof = false;
	if(span.duration() > total_duration()) throw std::invalid_argument("read duration larger than ring capacity");
	if(span.start_time() < read_start_time_) throw sequencing_error("time span to read already passed");
	if(reader_state_ != idle) throw sequencing_error("already reading");

	// skip frames if span does not start immediatly at first readable frame
	// may skip more frames than in buffer currently (skip() function) and wait for writer
	// to fill up buffer
	if(span.start_time() > read_start_time_)
		skip(span.start_time() - read_start_time_);	


	// lock the mutex: writer will no change write position or mark eof
	std::unique_lock<std::mutex> lock(mutex_);

	// reduce span if it crosses eof
	if(eof_was_marked()) {
		if(span.start_time() >= end_time_)
			throw sequencing_error("read span starts after eof");
	
		if(span.end_time() >= end_time_) {
			span = time_span(span.start_time(), end_time_);
			reaches_eof = true;
		}
	}

	// read position is now at start of span
	assert(read_start_time_ == span.start_time());

	// prevent deadlock (same as begin_write())
	if(ring_.readable_duration() < span.duration() && writer_state_ == waiting)
		throw sequencing_error("deadlock detected: ring buffer writer was already waiting");

	// wait until duration becomes readable
	while(ring_.readable_duration() < span.duration()) {
		reader_state_ = waiting;
		// wait until more frames written (mutex unlocked during wait, and relocked after)
		readable_cv_.wait(lock);
		assert(writer_state_ != waiting);
		
		// writer might have marked end while reader was waiting
		if(eof_was_marked() && span.end_time() >= end_time_) {
			span = time_span(span.start_time(), end_time_);
			reaches_eof = true;
		}
	}
	
	reader_state_ = accessing;
	
	return ring_.begin_read(span.duration());
}


template<std::size_t Dim, typename T>
auto ndarray_seekable_shared_ring<Dim, T>::begin_read_span(time_span span) -> section_view_type {
	bool reaches_eof;
	return begin_read_span(span, reaches_eof);
}


template<std::size_t Dim, typename T>
auto ndarray_seekable_shared_ring<Dim, T>::begin_read(time_unit duration, bool& reaches_eof) -> section_view_type {
	// read_start_time_ only changed by reader, so no need to block
	return begin_read_span(time_span(read_start_time_, read_start_time_ + duration), reaches_eof);
}


template<std::size_t Dim, typename T>
auto ndarray_seekable_shared_ring<Dim, T>::begin_read(time_unit duration) -> section_view_type {
	bool reaches_eof;
	return begin_read(duration, reaches_eof);
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
	writable_cv_.notify_one(); // ...notify condition variable: more writable frames have become available
}


template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::skip_available_(time_unit skip_duration) {
	{
		std::lock_guard<std::mutex> lock(mutex_);
		assert(skip_duration <= ring_.readable_duration()); // ...can only skip frames that are already readable
		ring_.skip(skip_duration);
		read_start_time_ += skip_duration;
	}
	writable_cv_.notify_one(); // ...notify condition variable: more writable frames have become available
}


template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::skip(time_unit skip_duration) {
	seek(read_start_time_ + skip_duration);
}


template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::seek(time_unit t) {
	if(t >= end_time_) throw std::invalid_argument("cannot seek beyond end");

	// lock the mutex
	// writer will not start or end while locked, and readable time span will not change
	std::unique_lock<std::mutex> lock(mutex_);
			
	bool already_readable = ring_.readable_time_span.includes(t);
	
	if(!already_readable && writer_state_ != idle) {
		// target tile span is not in buffer, and writer is not idle:
		// need to let the writer finish waiting&writing first 
	
		// first skip all readable frames in buffer
		ring_.skip(readable_span.duration());		

		// if writer was waiting, it will now continue when mutex is unlocked
		writable_cv_.notify_one();
		
		// allow writer to write its frames, and switch to idle state
		// works because end_write sets writer_state_ before notifying readable_cv_
		while(writer_state_ != idle) readable_cv_.wait(lock); // mutex gets unlocked while waiting
		// mutex is now locked again
		
		// retest if target time is now in readable span
		already_readable = ring_.readable_time_span.includes(t);
	}
	
	
	if(already_readable) {
		// target time is already in buffer: simply skip to it
		time_unit skip_duration = t - readable_span.start_time();
		ring_.skip(skip_duration);
		read_start_time_ += skip_duration;

		// notify writer: new writable frames
		lock.unlock();
		writable_cv_.notify_one();
						
	} else {
		// writer is necessarily idle now
		assert(writer_state_ == idle);
	
		// perform seek on ring buffer now
		ring_.seek(t);
		read_start_time_ = ring_.read_start_time();
		
		// no need to notify writer: it is idle
	}
}


template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::skip_span(time_span span) {
	time_unit skip_duration = span.start_time() - read_start_time_ + span.duration();
	skip(skip_duration);
}


template<std::size_t Dim, typename T>
time_unit ndarray_seekable_shared_ring<Dim, T>::current_time() const noexcept {
	return ring_.current_time();
}


template<std::size_t Dim, typename T>
time_unit ndarray_seekable_shared_ring<Dim, T>::write_start_time() const noexcept {
	// atomic, so no need to lock
	return ring_.write_start_time();
}


template<std::size_t Dim, typename T>
time_unit ndarray_seekable_shared_ring<Dim, T>::read_start_time() const noexcept {
	// use atomic member, instead of non-atomic ring_.read_start_time()
	return read_start_time_;
}


template<std::size_t Dim, typename T>
time_unit ndarray_seekable_shared_ring<Dim, T>::writable_duration() const {
	return writable_time_span().duration();
}


template<std::size_t Dim, typename T>
time_span ndarray_seekable_shared_ring<Dim, T>::writable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	time_span writable = ring_.writable_time_span();
	if(end_time_ == -1) return writable;
	else return time_span(writable.start_time(), std::min(writable.end_time(), end_time_.load()));
}


template<std::size_t Dim, typename T>
time_unit ndarray_seekable_shared_ring<Dim, T>::readable_duration() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.readable_duration();
}


template<std::size_t Dim, typename T>
time_span ndarray_seekable_shared_ring<Dim, T>::readable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.readable_time_span();
}


template<std::size_t Dim, typename T>
bool ndarray_seekable_shared_ring<Dim, T>::eof_was_marked() const {
	return (end_time_ != -1);
}


#ifndef NDEBUG
template<std::size_t Dim, typename T>
void ndarray_seekable_shared_ring<Dim, T>::debug_print(std::ostream& str) const {
	str << "ndarray_seekable_shared_ring: \n";
	if(mutex_.try_lock()) {
		ring_.debug_print(str);
		str << "reader state="
		    << (reader_state_ == idle ? "idle" : (reader_state_ == waiting ? "waiting" : "processing"))
		    << ", writer state="
		    << (writer_state_ == idle ? "idle" : (writer_state_ == writer_state_ ? "waiting" : "processing"))
		    << ", end_time=" << end_time_ << ", read_start_time=" << read_start_time_;
		mutex_.unlock();
	} else {
		str << "cannot lock mutex for debug_print" << std::endl;
	}
}
#endif


}
