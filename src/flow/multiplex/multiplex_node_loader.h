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

#ifndef MF_FLOW_MULTIPLEX_NODE_LOADER_H_
#define MF_FLOW_MULTIPLEX_NODE_LOADER_H_

#include "multiplex_node.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <atomic>

namespace mf { namespace flow {

class multiplex_node::loader {
private:
	multiplex_node& node_;
	thread_index thread_index_ = undefined_thread_index;

protected:
	multiplex_node& this_node() { return node_; }
	const multiplex_node& this_node() const { return node_; }

public:
	loader(multiplex_node&, thread_index);
	virtual ~loader() = default;
	
	thread_index loader_thread_index() const { return thread_index_; }
	virtual bool is_async() const = 0;
	
	virtual void stop() = 0;
	virtual void launch() = 0;
	virtual void pre_pull(time_span) = 0;
	virtual node::pull_result pull(time_span) = 0;
	virtual node_frame_window_view begin_read(time_span span) = 0;
	virtual void end_read(time_unit duration) = 0;
};


///////////////


class multiplex_node::sync_loader : public multiplex_node::loader {
public:
	explicit sync_loader(multiplex_node&);

	bool is_async() const override { return false; }

	void stop() override;
	void launch() override;
	void pre_pull(time_span) override;
	node::pull_result pull(time_span span) override;
	node_frame_window_view begin_read(time_span span) override;
	void end_read(time_unit duration) override;
};


///////////////


class multiplex_node::async_loader : public multiplex_node::loader {
private:
	std::thread thread_;
	std::atomic<bool> stopped_ {false};
	
	std::mutex successor_time_mutex_;	
	std::condition_variable successor_time_changed_cv_;

	std::shared_timed_mutex input_view_mutex_;
	std::condition_variable_any input_view_updated_cv_;

	void thread_main_();

public:
	explicit async_loader(multiplex_node&);
	~async_loader() override;

	bool is_async() const override { return true; }

	void stop() override;
	void launch() override;
	void pre_pull(time_span) override;
	node::pull_result pull(time_span span) override;
	node_frame_window_view begin_read(time_span span) override;
	void end_read(time_unit duration) override;
};

}}

#endif
