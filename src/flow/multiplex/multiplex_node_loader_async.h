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

#ifndef MF_FLOW_MULTIPLEX_NODE_LOADER_ASYNC_H_
#define MF_FLOW_MULTIPLEX_NODE_LOADER_ASYNC_H_

#include "multiplex_node.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <atomic>

namespace mf { namespace flow {

class multiplex_node::async_loader : public multiplex_node::loader {
private:
	std::thread thread_;
	std::atomic<bool> stop_;
	
	std::shared_timed_mutex input_mutex_;
	std::condition_variable_any input_cv_;
	time_unit input_fcs_time_ = -1;
	pull_result input_pull_result_ = pull_result::undefined;
	
	std::mutex request_mutex_;
	std::condition_variable request_cv_;
	time_unit request_fcs_time_ = -1;
	
	void thread_main_();

public:
	explicit async_loader(multiplex_node&);
	~async_loader() override;

	bool is_async() const override { return true; }

	void pre_stop() override;
	void stop() override;
	void launch() override;
	void pre_pull(time_span) override;
	node::pull_result pull(time_span&) override;
	node_frame_window_view begin_read(time_span) override;
	void end_read(time_unit duration) override;
};

}}

#endif
