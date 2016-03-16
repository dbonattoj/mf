#include "media_node_input.h"
#include <cassert>

namespace mf {


template<std::size_t Dim, typename T>
auto media_node_input<Dim, T>::connected_output() const -> output_type& {
	if(! is_connected()) throw std::logic_error("input is not connected to an output");
	return *connected_output_;
}


template<std::size_t Dim, typename T>
void media_node_input<Dim, T>::connect(output_type& output) {
	connected_output_ = &output;
}


template<std::size_t Dim, typename T>
void media_node_input<Dim, T>::begin_read(time_unit t) {
	if(connected_output_ == nullptr) throw std::logic_error("node input is not connected");
	
	// pull frames from connected output
	// recursively makes preceding nodes process frames (synchronously or asynchronously)
	connected_output_->pull(t + future_window_);
		
	// time span to request from connected output
	// if t near beginning, size of past window is reduced
	// remember view_center = position of current frame in window
	time_span requested_span;
	if(t < past_window_) {
		requested_span = time_span(0, t + future_window_ + 1);
		view_center_ = t;
	} else {
		requested_span = time_span(t - past_window_, t + future_window_ + 1);
		view_center_ = past_window_;
	}

	// begin read span from output's ring buffer
	// waits until it becomes available (for asynchronous nodes)
	auto view = connected_output_->begin_read_span(requested_span);
	
	// if near end, view duration < requested span duration, and size of future window is reduced
	assert(view.shape().front() <= requested_span.duration());
	view_.reset(view);
}


template<std::size_t Dim, typename T>
void media_node_input<Dim, T>::end_read(time_unit t) {
	if(connected_output_ == nullptr) throw std::logic_error("node input is not connected");
	
	// normal operation: 1 frame consumed from output = first from past window
	// if near beginning and past window still reduced: consume no frame
	connected_output_->end_read(t >= past_window_);
}


template<std::size_t Dim, typename T>
bool media_node_input<Dim, T>::reached_end() const {
	if(connected_output_ == nullptr) throw std::logic_error("node input is not connected");
	return connected_output_->reached_end();
}

}