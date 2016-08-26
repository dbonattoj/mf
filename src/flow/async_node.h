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

#include "processing_node.h"
#include "../queue/shared_ring.h"
#include <thread>

namespace mf { namespace flow {

class graph;

class async_node final : public processing_node {	
private:
	enum class process_result { should_continue, should_pause, failure };

	using request_id_type = int;
	
	time_unit prefetch_duration_ = 0;
	
	thread_index thread_index_ = undefined_thread_index;
	std::thread thread_;
	std::atomic<bool> running_ {false};

	std::unique_ptr<shared_ring> ring_;
	
	std::mutex continuation_mutex_;
	std::condition_variable continuation_cv_;
	std::atomic<time_unit> time_limit_ {-1};
	std::atomic<request_id_type> current_request_id_ {-1};
	std::atomic<bool> reconnect_flag_ {false};
	
	request_id_type failed_request_id_ = -1;
	
	bool pause_();
	process_result process_frame_();
	void thread_main_();

	pull_result output_pull_(time_span&, bool reconnected) override;
	timed_frame_array_view output_begin_read_(time_unit duration) override;
	void output_end_read_(time_unit duration) override;

public:
	async_node(graph&);
	~async_node() override;
	
	thread_index processing_thread_index() const override;
	
	time_unit prefetch_duration() const { return prefetch_duration_; }
	void set_prefetch_duration(time_unit dur) { prefetch_duration_ = dur; }
	
	void setup() override;
	void launch() override;
	void pre_stop() override;
	void stop() override;

	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
};


}}

#endif
