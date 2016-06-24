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
#include "../os/event.h"
#include <thread>

namespace mf { namespace flow {

class graph;
class async_node;


class async_node_output : public node_output {
private:
	async_node& this_node();
	const async_node& this_node() const;
	
public:
	using node_output::node_output;

	void setup() override;
	node::pull_result pull(time_span& span, bool reconnect) override;
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
	time_unit end_time() const override;
};



class async_node final : public filter_node {
private:
	using request_id_type = int;
	
	time_unit prefetch_duration_ = 0;
	
	std::thread thread_;

	std::unique_ptr<shared_ring> ring_;
	
	std::mutex continuation_mutex_;
	std::condition_variable continuation_cv_;
	std::atomic<time_unit> time_limit_ {-1};
	std::atomic<request_id_type> current_request_id_ {-1};
	std::atomic<bool> reconnect_flag_ {false};
	
	request_id_type failed_request_id_ = -1;
		
	async_node_output& output();
	const async_node_output& output() const;
	
	bool may_continue_();
	
	void thread_main_();
	bool process_frames_();
	
public:
	async_node(graph&);
	~async_node() override;
	
	void setup() override;
	void launch() override;
	void pre_stop() override;
	void stop() override;

	void output_setup();

	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
	
	void set_prefetch_duration(time_unit dur) { prefetch_duration_ = dur; }

	/// \name Remote output interface.
	///@{
	node::pull_result output_pull(time_span& span, bool reconnect);
	timed_frame_array_view output_begin_read(time_unit duration);
	void output_end_read(time_unit duration);
	///@}
	
	node_input& add_input(time_unit past_window, time_unit future_window) override {
		return add_input_<node_input>(past_window, future_window);
	}
	
	node_output& add_output(const frame_format& format) override;
};


}}

#endif
