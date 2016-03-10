namespace mf {

template<std::size_t Dim, typename T>
void ndarray_timed_ring<Dim, T>::end_write(std::size_t written_duration) {
	base::end_write(written_duration);
	last_write_time_ += written_duration;
}


template<std::size_t Dim, typename T>
time_span ndarray_timed_ring<Dim, T>::readable_time_span() const {
	return time_span(
		last_write_time_ + 1 - this->readable_duration(),
		last_write_time_ + 1
	);
}


template<std::size_t Dim, typename T>
time_span ndarray_timed_ring<Dim, T>::writable_time_span() const {
	return time_span(
		last_write_time_ + 1,
		last_write_time_ + 1 + this->writable_duration()
	);
}


template<std::size_t Dim, typename T>
auto ndarray_timed_ring<Dim, T>::begin_write_span(time_span span) -> section_view_type {
	// span must start immediately after the last written frame
	if(span.start_time() != last_write_time_ + 1)
		throw sequencing_error("time span to write must begin immediately after previous one");

	// return view to writable frames - may fail or wait, depending on subclass
	return this->begin_write(span.duration());
}


template<std::size_t Dim, typename T>	
auto ndarray_timed_ring<Dim, T>::begin_read_span(time_span span) -> section_view_type {
	// test duration, before doing any modification
	time_unit duration = span.duration();
	if(duration > base::total_duration()) throw std::invalid_argument("read duration too large");

	time_unit readable_start = readable_time_span().start_time();
	if(span.start_time() < readable_start) {
		// fail if span already (partially) read
		throw sequencing_error("time span to read already passed");
	} else if(span.start_time() > readable_start) {
		// skip frames until start of span - may fail or wait, depending on subclass
		this->skip(span.start_time() - readable_start);	
	}
	
	// ensure buffer is now at requested read position
	assert(readable_time_span().start_time() == span.start_time());

	// return view to readable frames - may fail or wait, depending on subclass
	return this->begin_read(duration);
}

template<std::size_t Dim, typename T>
void ndarray_timed_ring<Dim, T>::skip_span(time_span span) {
	time_unit readable_start = readable_time_span().start_time();
	time_unit skip_duration = span.start_time() - readable_start + span.duration();
	if(skip_duration > 0) this->skip(skip_duration);
	// does nothing when the span already fully passed
}



}
