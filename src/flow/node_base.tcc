#include <algorithm>
#include <iostream>

namespace mf { namespace flow {

template<std::size_t Dim, typename Elem>
bool node_base::output<Dim, Elem>::can_setup() const noexcept {
	return required_buffer_duration_is_defined() && frame_shape_is_defined();
}


template<std::size_t Dim, typename Elem>
void node_base::input<Dim, Elem>::connect(output_type& out) {
	connected_output_ = &out;
}


template<std::size_t Dim, typename Elem>
auto node_base::input<Dim, Elem>::connected_output() const -> output_type& {
	MF_EXPECTS(is_connected());
	return *connected_output_;
}


template<std::size_t Dim, typename Elem>
auto node_base::input<Dim, Elem>::view() -> frame_view_type {
	MF_EXPECTS(view_available_);
	time_unit t = node().current_time();
	return view_.at_time(node().current_time());
}


template<std::size_t Dim, typename Elem>
void node_base::output<Dim, Elem>::set_view(const frame_view_type& view) {
	MF_EXPECTS(! view_available_);
	view_.reset(view);
	view_available_ = true;
}


template<std::size_t Dim, typename Elem>
void node_base::output<Dim, Elem>::unset_view() {
	MF_EXPECTS(view_available_);
	view_available_ = false;
}


template<std::size_t Dim, typename Elem>
bool node_base::input<Dim, Elem>::reached_end(time_unit t) const {
	MF_EXPECTS(is_connected());

	time_unit output_end_time = connected_output().end_time();
	if(output_end_time != -1) {
		return ( t - past_window_duration() >= output_end_time );
	} else {
		return false;
	}
}


template<std::size_t Dim, typename Elem>
bool node_base::input<Dim, Elem>::begin_read_frame(time_unit t) {
	MF_EXPECTS(is_connected());
	MF_EXPECTS(! view_available_);
	MF_EXPECTS(! reached_end(t));
	
	// time span to request from connected output
	// truncate the past window if near beginning of stream
	time_span requested_span(
		std::max(time_unit(0), t - past_window_duration()),
		t + future_window_duration() + 1
	);

	// request span from connected output
	// depending on connected node type, may process frame(s) now (sync)
	// or wait for them to become available (async)
	// connected node seeks if necessary
	full_view_type view;
	bool status = connected_output().begin_read_span(requested_span, view);
	if(! status) return false;
	MF_ASSERT(view.start_time() == requested_span.start_time());
	if(! view.span().includes(t)) {
		connected_output().end_read(false);
		std::cout << "req: " << requested_span << " ,  span: " << view.span() << " ,  t=" << t << std::endl;
		throw sequencing_error("requested frame is after end of stream");
	}
	
	view_.reset(view);
	view_available_ = true;
	
	MF_ENSURES(view_available_);
	
	return true;
}


template<std::size_t Dim, typename Elem>
void node_base::input<Dim, Elem>::end_read_frame(time_unit t) {
	MF_EXPECTS(is_connected());
	MF_EXPECTS(view_available_);
	
	// when past window was truncated begin_read_frame(), the first frame in the window will
	// also still be in the next window for t + 1. (unless seek occurs next)
	// --> then it is not consumed
	bool consume_frame = (t >= past_window_duration());
	MF_ASSERT(view_.start_time() == std::max(time_unit(0), t - past_window_duration()));
	
	connected_output().end_read(consume_frame);
	view_available_ = false;
		
	MF_EXPECTS(! view_available_);
}


}}
