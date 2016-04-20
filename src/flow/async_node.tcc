namespace mf { namespace flow {

template<std::size_t Dim, typename Elem>
void async_node::output<Dim, Elem>::setup() {
	MF_EXPECTS(base::required_buffer_duration_is_defined());
	MF_EXPECTS(base::frame_shape_is_defined());
	
	ring_.reset(new ring_type(
		base::frame_shape(),
		base::required_buffer_duration(),
		base::node().is_seekable(),
		base::node().stream_duration()
	));
}


template<std::size_t Dim, typename Elem>
time_unit async_node::output<Dim, Elem>::begin_write_next_frame() {
	MF_EXPECTS(! base::view_is_available());
	
	auto view = ring_->begin_write(1);
	if(view.duration() == 0) {
		return -1;
	} else {
		MF_ASSERT(view.duration() == 1);
		base::set_view(view[0]);
		return view.start_time();
	}
}


template<std::size_t Dim, typename Elem>
void async_node::output<Dim, Elem>::end_write_frame(bool mark_end) {
	MF_EXPECTS(base::view_is_available());
	
	ring_->end_write(1, !ring_->is_seekable() && mark_end);
	base::unset_view();
	
	MF_ENSURES(! base::view_is_available());
}


template<std::size_t Dim, typename Elem>
void async_node::output<Dim, Elem>::cancel_write_frame() {
	MF_EXPECTS(base::view_is_available());
	
	ring_->end_write(0);
	base::unset_view();
	
	MF_ENSURES(! base::view_is_available());
}


template<std::size_t Dim, typename Elem>
auto async_node::output<Dim, Elem>::begin_read_span(time_span span) -> full_view_type {
	async_node& nd = dynamic_cast<async_node&>(base::node()); // TODO change
	nd.time_limit_ = span.end_time() + nd.prefetch_duration();

	auto view = ring_->begin_read_span(span);
	MF_ASSERT(view.duration() <= span.duration());
	return view;
}


template<std::size_t Dim, typename Elem>
void async_node::output<Dim, Elem>::end_read(bool consume_frame) {
	ring_->end_read(consume_frame ? 1 : 0);
}


template<std::size_t Dim, typename Elem>
void async_node::output<Dim, Elem>::skip_frame() {
	ring_->skip(1);
}


template<std::size_t Dim, typename Elem>
time_unit async_node::output<Dim, Elem>::end_time() const {
	return ring_->end_time();
}


}}
