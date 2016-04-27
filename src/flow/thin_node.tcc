namespace mf { namespace flow {

template<std::size_t Dim, typename Elem>
void thin_node::output<Dim, Elem>::setup() {

}


template<std::size_t Dim, typename Elem>
bool thin_node::output<Dim, Elem>::begin_write_next_frame(time_unit& t) {

}


template<std::size_t Dim, typename Elem>
void thin_node::output<Dim, Elem>::end_write_frame(bool mark_end) {

}


template<std::size_t Dim, typename Elem>
void thin_node::output<Dim, Elem>::cancel_write_frame() {

}


template<std::size_t Dim, typename Elem>
bool thin_node::output<Dim, Elem>::begin_read_span(time_span span, full_view_type& view) {	
	thin_node& nd = dynamic_cast<thin_node&>(base::node()); // TODO change

	MF_EXPECTS(span.duration() == 1);
	
	auto processed_view = nd.process_frame_(span.start_time());
	view.reset(processed_view.start(), make);
	
	return nd.process_frame_(span.start_time(), view);
}


template<std::size_t Dim, typename Elem>
void thin_node::output<Dim, Elem>::end_read(bool consume_frame) {

}


template<std::size_t Dim, typename Elem>
time_unit thin_node::output<Dim, Elem>::end_time() const {

}


}}
