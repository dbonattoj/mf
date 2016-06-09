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

#ifndef MF_FLOW_ASYNC_NODE_H_
#define MF_FLOW_ASYNC_NODE_H_

#include "filter_node.h"
#include "../queue/shared_ring.h"
#include <thread>

namespace mf { namespace flow {

class graph;
class async_node;


class async_node_output : public node_output {
public:
	std::unique_ptr<shared_ring> ring_;

	time_span allowed_span_;

public:
	using node_type = async_node;

	using node_output::node_output;

	void setup() override;

	/// \name Read interface, used by connected input.
	///@{
	bool pull(time_span span, bool reconnected) override;
	
	/// Begin reading `duration` frames pulled previously.
	/** For asynchronous views, thje function waits until the frames become available (or end of stream occurs), and
	 ** returns the view with duration `0` to `duration` (lower than `duration` if end was reached.). If stream was
	 ** interrupted, returns null view. */
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
	time_unit end_time() const override;
	///@}
	
	/// \name Write interface, used by node.
	///@{
	frame_view begin_write_frame(time_unit& t) override;
	void end_write_frame(bool was_last_frame) override;
	void cancel_write_frame() override;
	///@}
};



/// Asynchronous node base class.
/** Processes frames in a separate thread owned by the node. Can have multiple inputs, but only one output. Can process
 ** frames `t+k` (`k <= 1`), at the same time that current frame `t` is being read or processed by suceeding nodes
 ** in graph. */
class async_node final : public filter_node {
public:
	bool running_ = false;
	std::thread thread_;
	
	std::atomic<bool> reconnect_flag_ {false};
	
	void thread_main_();
			
public:
	explicit async_node(graph&);
	~async_node();
	
	void setup() final override;
	void launch() final override;
	void stop() final override;
	bool process_next_frame() override;
	
	node_input& add_input(time_unit past_window, time_unit future_window) override {
		return add_input_<node_input>(past_window, future_window);
	}
	
	async_node_output& add_output(const frame_format& format) override {
		return add_output_<async_node_output>(format);
	}
};

}}

#endif
