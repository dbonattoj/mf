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

#ifndef MF_FLOW_NODE_INPUT_H_
#define MF_FLOW_NODE_INPUT_H_

#include "node.h"
#include "node_output.h"

namespace mf { namespace flow {

/// Input port of node in flow graph.
class node_input {
private:
	node& node_;

	time_unit past_window_ = 0;
	time_unit future_window_ = 0;
	
	node_output* connected_output_ = nullptr;
	
	time_span pulled_span_;
	bool activated_ = true;
			
public:
	explicit node_input(node& nd);
	node_input(const node_input&) = delete;
	node_input& operator=(const node_input&) = delete;
	virtual ~node_input() { }

	node& this_node() const noexcept { return node_; }

	void set_past_window(time_unit dur) { past_window_ = dur; }
	void set_future_window(time_unit dur) { future_window_ = dur; }

	time_unit past_window_duration() const noexcept { return past_window_; }
	time_unit future_window_duration() const noexcept { return future_window_; }
	
	void connect(node_output& out);
	void disconnect();
	bool is_connected() const noexcept { return (connected_output_ != nullptr); }
	node_output& connected_output() const
		{ Assert(is_connected()); return *connected_output_; }
	node& connected_node() const
		{ Assert(is_connected()); return connected_output().this_node(); }

	std::size_t channels_count() const
		{ Assert(is_connected()); return connected_output().channels_count(); }

	bool is_activated() const noexcept { return activated_; }
	void set_activated(bool);
	
	/// \name Read interface, used by node.
	///@{
	node::pull_result pull();
	const time_span& pulled_span() const noexcept { return pulled_span_; }
	timed_frame_array_view begin_read_frame();
	void end_read_frame();
	void cancel_read_frame();
	time_unit end_time() const { return connected_output_->end_time(); }
	///@}
};

}}

#endif
