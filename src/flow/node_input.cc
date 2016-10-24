/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "node_input.h"
#include "node_graph.h"

namespace mf { namespace flow {

node_input::node_input(node& nd) :
	node_(nd) { }


void node_input::connect(node_output& output) {
	Assert(connected_output_ == nullptr, "cannot connect node_input when already connected");
	Assert(! output.is_connected(), "cannot connect the input to an output that is already connected");
	connected_output_ = &output;
	connected_output_->input_has_connected(*this);
}


void node_input::disconnect() {
	Assert(connected_output_ != nullptr, "cannot disconnect node_input when not connected");
	connected_output_->input_has_disconnected();
	connected_output_ = nullptr;
}


time_span node_input::current_required_time_span_() const {
	time_unit t = this_node().current_time();
	time_unit start_time = std::max(time_unit(0), t - past_window_);
	time_unit end_time = t + future_window_ + 1;
	return time_span(start_time, end_time);
}


void node_input::pre_pull() {
	Assert(is_connected());
	time_span span = current_required_time_span_();
	connected_output_->pre_pull(span);
}



node::pull_result node_input::pull() {
	Assert(is_connected());
	
	if(this_node().graph().was_stopped()) return node::pull_result::stopped;
	
	time_unit t = this_node().current_time();
	time_span expected_span = current_required_time_span_();
	time_span span = expected_span;
				//MF_DEBUG_EXPR_T("node", t, span, past_window_, future_window_);
	
	node::pull_result result = connected_output_->pull(span); // span gets modified
	pulled_span_ = span;

	MF_DEBUG_T("node", "in.pull.  pulling ", expected_span, "  got  ", span, "   t=", t, "   result=", (int)result);

	
	if(result == node::pull_result::end_of_stream) {
		// If connected output reached end of stream, return error state only if frame t was not pulled
		// Future window is allowed to be truncated when nearing end of stream
		if(! span.includes(t)) return node::pull_result::end_of_stream;
		else return node::pull_result::success;
	}
	
	return result;
}


node_frame_window_view node_input::begin_read_frame() {
	Assert(pulled_span_.duration() > 0);

	// Truncate duration to read (from time window)
	time_unit t = this_node().current_time();
	time_unit duration = std::min(t, past_window_) + 1 + future_window_;
	if(duration > pulled_span_.duration()) duration = pulled_span_.duration();
		
	timed_frame_array_view view = connected_output_->begin_read(duration);
	if(view.is_null()) return view;
	
	MF_DEBUG_T("node", "in.begin_read_frame.  pulled ", pulled_span_, "   t=", t, "  got_span: ", view.span(), "  view.span().includes(t)=", view.span().includes(t));
	
	Assert(view.span().includes(t));
	Assert(view.start_time() == pulled_span_.start_time());
	Assert(view.duration() == duration);
	return view;
}


void node_input::end_read_frame() {
	// Normal case: mark 1 frame as having been read
	// When reading sequentially (no seek), the first frame of the pulled span won't be included in the next pulled
	// span. The next pulled span will start at this pulled span's start time + 1. So it will read sequentially from
	// the ring buffer in the connected output.
	
	// When past window was truncated, the next sequential pulled span will still start at t=0. So don't mark first
	// frame as having been read, otherwise the connected node will need to seek back.
	time_unit duration = (this_node().current_time() < past_window_) ? 0 : 1;
	connected_output_->end_read(duration);
}


void node_input::cancel_read_frame() {
	connected_output_->end_read(0);
}


void node_input::set_activated(bool activated) {
	if(activated_ != activated) {
		activated_ = activated;
	}
}

}}
