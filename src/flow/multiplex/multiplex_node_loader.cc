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

multiplex_node::loader::loader(multiplex_node& nd, thread_index tind) :
	node_(nd),
	thread_index_(tind) { }
	

///////////////


multiplex_node::sync_loader::sync_loader(multiplex_node& nd) :
	loader(nd, nd.output_at(0).reader_thread_index())
{
	Assert(! nd.outputs_on_different_threads_(),
		"multiplex_node with sync_loader must have all outputs on same reader thread");
}


void multiplex_node::sync_loader::stop() { }


void multiplex_node::sync_loader::launch() { }


void multiplex_node::sync_loader::pre_pull(time_span span) { }


node::pull_result multiplex_node::sync_loader::pull(time_span& span) {
	// get expected input span, based on current time of the common successor node
	time_unit successor_time = this_node().capture_successor_time_();
	time_span expected_input_span = this_node().expected_input_span_(successor_time);
	
	if(! expected_input_span.includes(span)) {
		// tfail if pulled span not in this expected span
		span.set_end_time(span.start_time());
		return pull_result::transitory_failure;
	}
	
	
	if(successor_time != this_node().successor_time_of_input_view_()) {
		// reload input view if successor_time has changed since last time
		input_pull_result_ = this_node().load_input_view_(successor_time);
		Assert(this_node().current_time() == successor_time);
	}

	time_span loaded_input_span = this_node().input_view_().span();
	span = span_intersection(span, loaded_input_span);

	// TODO verify
	if(input_pull_result_ != pull_result::success) return input_pull_result_;
	else if(loaded_input_span.start_time() != expected_input_span.start_time()) return pull_result::transitory_failure;
	else if(loaded_input_span.includes(span)) return pull_result::success;
	else return pull_result::end_of_stream;
}

//	enum class pull_result { success, fatal_failure, transitory_failure, stopped, end_of_stream };



node_frame_window_view multiplex_node::sync_loader::begin_read(time_span span) {
	std::cout << "loaded span: " << this_node().input_view_().span() << std::endl;
	std::cout << "reques span: " << span << std::endl;
	
	
	timed_frame_array_view input_view = this_node().input_view_();
	return input_view.time_section(span);
}


void multiplex_node::sync_loader::end_read(time_unit duration) { }


///////////////


multiplex_node::async_loader::async_loader(multiplex_node& nd) :
	loader(nd, nd.graph().new_thread_index()),
	stopped_(false) { }


multiplex_node::async_loader::~async_loader() {
	Assert(! thread_.joinable(), "multiplex_node::async_loader must be stopped before destruction");
}


void multiplex_node::async_loader::thread_main_() {
	for(;;) {
		// will hold time of common successor node for which new input is loaded
		time_unit successor_time = -1;
		
		// wait until current successor_time no longer equal to the successor time for which input view was loaded
		// needs to be awakened by reader by notifying successor_time_changed_cv_
		std::unique_lock<std::mutex> lock(successor_time_mutex_);
		successor_time_changed_cv_.wait(lock, [&] {
			if(stopped_) return true;
			successor_time = this_node().capture_successor_time_();
			return (successor_time != this_node().successor_time_of_input_view_());
		});
		lock.unlock();
		if(stopped_) break;
		Assert(successor_time != -1);
		
		// acquire exclusive lock on input view mutex --> waits until all readers end
		// and load the new input view, and keep response from input pull
		{
			std::lock_guard<std::shared_timed_mutex> view_lock(input_view_mutex_);
			input_pull_result_ = this_node().load_input_view_(successor_time);
		}
		
		// notify waiting readers that input view + input pull response has changed
		input_view_updated_cv_.notify_all();
	}
	
	// loader was stopped:
	// need to unload input view (so as to end reading from input)
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
	// TODO prove formally
		
	// get expected input span, based on current time of the common successor node
	time_unit successor_time = this_node().capture_successor_time_();
	time_span expected_input_span = this_node().expected_input_span_(successor_time);
	if(! expected_input_span.includes(span)) {
		// tfail if pulled span not in this expected span
		return node::pull_result::transitory_failure;
	}
	
	{
		// acquire shared lock on input view
		// shared with other readers calling pull() on different threads
		// loader can modify the input view when all readers release the shared lock
		std::shared_lock<std::shared_timed_mutex> lock(input_view_mutex_);
		
		// wail until loader updates the input view for the current time of the common successor node
		while(this_node().current_time() != this_node().capture_successor_time_()) {
			if(stopped_) return node::pull_result::stopped;
			successor_time_changed_cv_.notify_one();
			input_view_updated_cv_.wait(lock);
		}
		if(stopped_) return node::pull_result::stopped;
	
		pull_result load_res = input_pull_result_;
		time_span loaded_input_span = this_node().input_view_().span();
		Assert(this_node().current_time() == successor_time);
	
		if(load_res != pull_result::success) return load_res;
		else if(loaded_input_span.start_time() != expected_input_span.start_time()) return pull_result::transitory_failure;
		else if(loaded_input_span.includes(span)) return pull_result::success;
		else return pull_result::end_of_stream;
	}
}




node_frame_window_view multiplex_node::async_loader::begin_read(time_span span) {
	std::shared_lock<std::shared_timed_mutex> lock(input_view_mutex_);
	
	timed_frame_array_view input_view = this_node().input_view_();
	return input_view.time_section(span);
}


void multiplex_node::async_loader::end_read(time_unit duration) {
	input_view_mutex_.unlock_shared();
}



}}
