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

#include "multiplex_node_loader.h"
#include "../node_graph.h"
#include <utility>
#include <functional>

namespace mf { namespace flow {


multiplex_node::async_loader::async_loader(multiplex_node& nd) :
	loader(nd, nd.graph().new_thread_index()),
	stopped_(false) { }


multiplex_node::async_loader::~async_loader() {
	Assert(! thread_.joinable(), "multiplex_node::async_loader must be stopped before destruction");
}


void multiplex_node::async_loader::thread_main_() {
	for(;;) {
		time_unit successor_time = -1;
		
		{
			std::unique_lock<std::mutex> lock(successor_time_mutex_);
		
			do {
				successor_time_changed_cv_.wait(lock);
				successor_time = input_successor_time_;
			} while(successor_time == input_successor_time_ && !stopped_);
			
			if(stopped_) break;
		}


		Assert(successor_time != -1);
		
		{
			std::lock_guard<std::shared_timed_mutex> view_lock(input_view_mutex_);
			input_pull_result_ = this_node().load_input_view_(successor_time);
			input_successor_time_ = successor_time;
		}
		
		input_view_updated_cv_.notify_all();
	}
	
	{
		std::lock_guard<std::shared_timed_mutex> view_lock(input_view_mutex_);
		this_node().unload_input_view_();
		input_pull_result_ = node::pull_result::stopped;
	}
}


//enum class pull_result { success, fatal_failure, transitory_failure, stopped, end_of_stream };


void multiplex_node::async_loader::stop() {
	Assert(this_node().graph().was_stopped());
	Assert(thread_.joinable());
	
	stopped_ = true;

	successor_time_changed_cv_.notify_one();
	input_view_updated_cv_.notify_all();
	
	thread_.join();	
}


void multiplex_node::async_loader::launch() {
	stopped_ = false;
	thread_ = std::move(std::thread(
		std::bind(&multiplex_node::async_loader::thread_main_, this)
	));
}


void multiplex_node::async_loader::pre_pull(time_span span) {		
	if(this_node().current_time() != this_node().capture_successor_time_()) {
		if(stopped_) return;
		successor_time_changed_cv_.notify_one();
	}
}


node::pull_result multiplex_node::async_loader::pull(time_span& span) {
	time_unit successor_time = this_node().capture_successor_time_();
	time_span expected_input_span = this_node().expected_input_span_(successor_time);
	if(! expected_input_span.includes(span)) return node::pull_result::transitory_failure;
	
	{
		std::shared_lock<std::shared_timed_mutex> lock(input_view_mutex_);

		auto receive_new_input_view = [&]() {
			{
				std::lock_guard<std::mutex> lock(successor_time_mutex_);
				input_successor_time_ = this_node().capture_successor_time_();
			}
			input_view_updated_cv_.wait(lock);
		};
	
		if(input_pull_result_ != pull_result::success) {
			receive_new_input_view();
		}
		
		while(input_pull_result_ == pull_result::success && input_successor_time_ != this_node().capture_successor_time_()) {
			receive_new_input_view();
		}
		
		pull_result load_res = input_pull_result_;
		time_span loaded_input_span = this_node().input_view_().span();
		span = span_intersection(span, loaded_input_span);
		Assert(input_successor_time_ == this_node().capture_successor_time_());


		if(load_res != pull_result::success) return load_res;
		else if(loaded_input_span.includes(span)) return pull_result::success;
		else if(loaded_input_span.start_time() != expected_input_span.start_time()) return pull_result::transitory_failure;
		else return pull_result::end_of_stream;
	}
}



node_frame_window_view multiplex_node::async_loader::begin_read(time_span span) {	
	input_view_mutex_.lock_shared();
	
	timed_frame_array_view input_view = this_node().input_view_();
	return input_view.time_section(span);
}


void multiplex_node::async_loader::end_read(time_unit duration) {
	input_view_mutex_.unlock_shared();
}



}}
