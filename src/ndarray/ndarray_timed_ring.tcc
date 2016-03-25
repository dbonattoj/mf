namespace mf {

template<std::size_t Dim, typename T>
void ndarray_timed_ring<Dim, T>::initialize() {
	last_write_time_ = -1;
	base::initialize();
}


template<std::size_t Dim, typename T>
void ndarray_timed_ring<Dim, T>::end_write(std::size_t written_duration) {
	base::end_write(written_duration);
	last_write_time_ += written_duration;
}


template<std::size_t Dim, typename T>
time_span ndarray_timed_ring<Dim, T>::readable_time_span() const {
	return time_span(
		last_write_time_ + 1 - base::readable_duration(),
		last_write_time_ + 1
	);
}


template<std::size_t Dim, typename T>
time_span ndarray_timed_ring<Dim, T>::writable_time_span() const {
	return time_span(
		last_write_time_ + 1,
		last_write_time_ + 1 + base::writable_duration()
	);
}


template<std::size_t Dim, typename T>
time_unit ndarray_timed_ring<Dim, T>::read_start_time() const noexcept {
	return last_write_time_ + 1 - base::readable_duration();
}


template<std::size_t Dim, typename T>
time_unit ndarray_timed_ring<Dim, T>::write_start_time() const noexcept {
	return last_write_time_ + 1;
}


template<std::size_t Dim, typename T>
bool ndarray_timed_ring<Dim, T>::can_write_span(time_span span) const {
	if(span.start_time() != last_write_time_ + 1) return false; // must start immediately after the last written frame 
	else if(span.duration() > base::writable_duration()) return false;
	else return true;
}


template<std::size_t Dim, typename T>
bool ndarray_timed_ring<Dim, T>::can_read_span(time_span span) const {
	time_unit skip_and_read_duration = span.start_time() - read_start_time() + span.duration();
	if(span.start_time() < read_start_time()) return false; // time span to read already passed
	else if(skip_and_read_duration > base::readable_duration()) return false;
	else return true;
}


template<std::size_t Dim, typename T>
bool ndarray_timed_ring<Dim, T>::can_skip_span(time_span span) const {
	time_unit skip_duration = span.start_time() - read_start_time() + span.duration();
	if(skip_duration > base::readable_duration()) return false;
	else return true;
}



template<std::size_t Dim, typename T>
auto ndarray_timed_ring<Dim, T>::begin_write_span(time_span span) -> section_view_type {
	if(span.duration() > base::total_duration()) throw std::invalid_argument("write duration larger than ring capacity");
	if(! can_write_span(span)) throw sequencing_error("cannot write span");
	return base::begin_write(span.duration());
}


template<std::size_t Dim, typename T>	
auto ndarray_timed_ring<Dim, T>::begin_read_span(time_span span) -> section_view_type {
	if(span.duration() > base::total_duration()) throw std::invalid_argument("read duration larger than ring capacity");
	if(! can_read_span(span)) throw sequencing_error("cannot read span");
	time_unit read_start = read_start_time();
	if(span.start_time() > read_start)
		base::skip(span.start_time() - read_start);	

	assert(readable_time_span().start_time() == span.start_time());
	return base::begin_read(span.duration());
}


template<std::size_t Dim, typename T>
void ndarray_timed_ring<Dim, T>::skip_span(time_span span) {
	if(span.duration() > base::total_duration()) throw std::invalid_argument("skip duration larger than ring capacity");
	if(! can_skip_span(span)) throw sequencing_error("cannot skip span");
	
	time_unit skip_duration = span.start_time() - read_start_time() + span.duration();
	if(skip_duration > 0) base::skip(skip_duration);
}


#ifndef NDEBUG
template<std::size_t Dim, typename T>
void ndarray_timed_ring<Dim, T>::debug_print(std::ostream& str) const {
	str << "ndarray_timed_ring: \n";
	base::debug_print(str);
	str << "last_write_time=" << last_write_time_ << ", readable_time_span=" << readable_time_span()
	    << ", writable_time_span=" << writable_time_span() << std::endl;
	
}
#endif


}
