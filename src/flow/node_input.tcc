#include "node_input.h"
#include <cassert>

namespace mf { namespace flow {


template<std::size_t Dim, typename T>
auto node_input<Dim, T>::connected_output() const -> output_type& {
	if(! is_connected()) throw std::logic_error("input is not connected to an output");
	return *connected_output_;
}


template<std::size_t Dim, typename T>
void node_input<Dim, T>::connect(output_type& output) {
	connected_output_ = &output;
}


template<std::size_t Dim, typename T>
void node_input<Dim, T>::begin_read(time_unit t) {	
	MF_DEBUG("input::begin_read(time = ", t, ")....");

	if(connected_output_ == nullptr) throw std::logic_error("node input is not connected");
	
	time_span requested_span;
	if(t < past_window_) {
		requested_span = time_span(0, t + future_window_ + 1);
		view_center_ = t;
	} else {
		requested_span = time_span(t - past_window_, t + future_window_ + 1);
		view_center_ = past_window_;
	}

	auto view = connected_output_->ring().begin_read_span(requested_span);
	assert(view.shape().front() <= requested_span.duration());
	view_.reset(view);
	view_available_ = true;
	
	MF_DEBUG("input::begin_read(time = ", t, ") --> ", view.span());
}


template<std::size_t Dim, typename T>
void node_input<Dim, T>::end_read(time_unit t) {
	if(connected_output_ == nullptr) throw std::logic_error("node input is not connected");
		
	bool consume_frame = (t >= past_window_);
	connected_output_->ring().end_read(consume_frame ? 1 : 0);
	view_available_ = false;

	MF_DEBUG("input::end_read(time = ", t, ")");
}


template<std::size_t Dim, typename T>
bool node_input<Dim, T>::reached_end() const {
	if(connected_output_ == nullptr) throw std::logic_error("node input is not connected");

	MF_DEBUG("reader_reached_end=", connected_output_->ring().reader_reached_end(), " - \n output:=", *connected_output_);

	if(connected_output_->ring().end_time_is_defined()) {
		if(connected_output_->ring().reader_reached_end()) return true;
		else return (connected_output_->ring().end_time() - past_window_) <= connected_output_->ring().read_start_time();
	}
	return false;
}

}}
