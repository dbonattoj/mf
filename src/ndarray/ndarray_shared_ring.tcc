namespace mf {

template<std::size_t Dim, typename T>
ndarray_shared_ring<Dim, T>::ndarray_shared_ring(const ndsize<Dim>& frames_shape, std::size_t duration) :
	ring_(frames_shape, duration) { }


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::initialize() {
	std::lock_guard<std::mutex> lock(mutex_);
	ring_.initialize();
	reader_state_ = idle;
	writer_state_ = idle;
	end_time_ = -1;
	read_start_time_ = 0;
}


template<std::size_t Dim, typename T>
auto ndarray_shared_ring<Dim, T>::begin_write_span(time_span span) -> section_view_type {
	if(span.start_time() != ring_.current_time() + 1)
		throw sequencing_error("time span to write must begin immediately after previous one");

	return begin_write(span.duration());
}


template<std::size_t Dim, typename T>
auto ndarray_shared_ring<Dim, T>::begin_write(time_unit duration) -> section_view_type {
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

		// no need to recheck if reader is waiting:
		// writer_state_ remained waiting so reader couldn't have started waiting
		assert(reader_state_ != waiting);
	}

	// mutex_ is still locked
	writer_state_ = accessing;
	return ring_.begin_write(duration);
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::end_write(time_unit written_duration, bool mark_eof) {
	if(writer_state_ == idle) throw sequencing_error("was not writing");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if(mark_eof)
			end_time_ = ring_.writable_time_span().start_time() + written_duration;
		ring_.end_write(written_duration);
	}

	writer_state_ = idle;
	readable_cv_.notify_one();	
}


template<std::size_t Dim, typename T>
auto ndarray_shared_ring<Dim, T>::begin_read_span(time_span span, bool& reaches_eof) -> section_view_type
{
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
auto ndarray_shared_ring<Dim, T>::begin_read_span(time_span span) -> section_view_type {
	bool reaches_eof;
	return begin_read_span(span, reaches_eof);
}


template<std::size_t Dim, typename T>
auto ndarray_shared_ring<Dim, T>::begin_read(time_unit duration, bool& reaches_eof) -> section_view_type {
	// read_start_time_ only changed by reader, so no need to block
	return begin_read_span(time_span(read_start_time_, read_start_time_ + duration), reaches_eof);
}


template<std::size_t Dim, typename T>
auto ndarray_shared_ring<Dim, T>::begin_read(time_unit duration) -> section_view_type {
	bool reaches_eof;
	return begin_read(duration, reaches_eof);
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::end_read(time_unit read_duration) {
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
void ndarray_shared_ring<Dim, T>::skip_available_(time_unit skip_duration) {
	{
		std::lock_guard<std::mutex> lock(mutex_);
		assert(skip_duration <= ring_.readable_duration()); // ...can only skip frames that are already readable
		ring_.skip(skip_duration);
		read_start_time_ += skip_duration;
	}
	writable_cv_.notify_one(); // ...notify condition variable: more writable frames have become available
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::skip(time_unit skip_duration) {
	// duration may be larger than ring capacity
		
	const auto initial_readable_duration = readable_duration(); // locks mutex_
	
	// mutex_ briefly unlocked, but readable duration will not decrease
	
	if(skip_duration <= initial_readable_duration) {
		// simple case: only skip some of the already readable frames
		skip_available_(skip_duration);

	} else {
		auto remaining_skip_duration = skip_duration;
			
		// wait for whole buffer to fill up and then skip all frames
		// until less than buffer size remains to be skipped
		while(remaining_skip_duration >= total_duration()) {
			bool reached_eof;
			auto readable_view = begin_read(total_duration(), reached_eof); // ...wait for buffer to fill up entirely
			if(! reached_eof) {
				// skip all and notify writable_cv_
				skip_available_(total_duration());
				remaining_skip_duration -= total_duration();
				reader_state_ = idle;
			} else {
				// eof was marked, don't try to skip beyond
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
		// if >1 frames get written at a time
	}
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::skip_span(time_span span) {
	time_unit skip_duration = span.start_time() - read_start_time_ + span.duration();
	skip(skip_duration);
}


template<std::size_t Dim, typename T>
time_unit ndarray_shared_ring<Dim, T>::current_time() const noexcept {
	return ring_.current_time();
}


template<std::size_t Dim, typename T>
time_unit ndarray_shared_ring<Dim, T>::write_start_time() const noexcept {
	// atomic, so no need to lock
	return ring_.write_start_time();
}


template<std::size_t Dim, typename T>
time_unit ndarray_shared_ring<Dim, T>::read_start_time() const noexcept {
	// use atomic member, instead of non-atomic ring_.read_start_time()
	return read_start_time_;
}


template<std::size_t Dim, typename T>
time_unit ndarray_shared_ring<Dim, T>::writable_duration() const {
	return writable_time_span().duration();
}


template<std::size_t Dim, typename T>
time_span ndarray_shared_ring<Dim, T>::writable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	time_span writable = ring_.writable_time_span();
	if(end_time_ == -1) return writable;
	else return time_span(writable.start_time(), std::min(writable.end_time(), end_time_.load()));
}


template<std::size_t Dim, typename T>
time_unit ndarray_shared_ring<Dim, T>::readable_duration() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.readable_duration();
}


template<std::size_t Dim, typename T>
time_span ndarray_shared_ring<Dim, T>::readable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.readable_time_span();
}


template<std::size_t Dim, typename T>
bool ndarray_shared_ring<Dim, T>::eof_was_marked() const {
	return (end_time_ != -1);
}


}
