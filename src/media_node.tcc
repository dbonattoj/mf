namespace mf {

template<std::size_t Dim, typename T>
void media_node_input<Dim, T>::pull(time_unit t) {
	if(connected_output_ == nullptr)
		throw std::logic_error("node input is not connected");
	time_span span = requested_span_(t);
	connected_output_->pull(span);
}


template<std::size_t Dim, typename T>
void media_node_input<Dim, T>::begin_read(time_unit t) {
	if(connected_output_ == nullptr)
		throw std::logic_error("node input is not connected");
	time_span span = requested_span_(t);
	auto view = connected_output_->begin_read(span);
	view_.reset(view);
	assert(view_.shape()[0] == span.duration());
}


template<std::size_t Dim, typename T>
void media_node_input<Dim, T>::end_read(time_unit t) {
	if(connected_output_ == nullptr)
		throw std::logic_error("node input is not connected");
	connected_output_->end_read(t < past_window_ ? 0 : 1);
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::pull(time_span span) {
	time_unit last_frame = span.end_time();
	while(node_.current_time() < last_frame) {
		node_.pull_frame();
	}
	
	MF_DEBUG("readable=", buffer_.readable_time_span(), "\nrequested=", span);
	
	if(! buffer_.readable_time_span().includes(span))
		throw sequencing_error("full requested span is not readable in output buffer");
}


template<std::size_t Dim, typename T>
auto media_node_output<Dim, T>::begin_read(time_span span) -> read_view_type {
	return buffer_.begin_read_span(span);
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::end_read(time_unit t) {
	buffer_.end_read(t);
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::begin_write() {
	auto view = buffer_.begin_write(1);
	view_.reset(view[0]);
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::end_write() {
	buffer_.end_write(1);
}



}