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

#include "multiplex_node_loader_async.h"
#include "../node_graph.h"
#include <utility>
#include <functional>
#include "../../os/thread.h"

namespace mf { namespace flow {

multiplex_node::async_loader::async_loader(multiplex_node& nd) :
	loader(nd, nd.graph().new_thread_index()),
	stop_(false) { }


multiplex_node::async_loader::~async_loader() {
	Assert(! thread_.joinable(), "multiplex_node::async_loader must be stopped before destruction");
}


void multiplex_node::async_loader::thread_main_() {
	for(;;) {
		{
			std::unique_lock<std::mutex> lock(request_mutex_);
			request_cv_.wait(lock, [&] { return stop_ || (input_fcs_time_ != request_fcs_time_); });
		}
		
		{
			std::lock_guard<std::shared_timed_mutex> in_lock(input_mutex_);
			MpxDebug("inpull req=", request_fcs_time_, "  in=", input_fcs_time_);
			input_pull_result_ = this_node().load_input_view_(request_fcs_time_);
			MpxDebug("inpull req=", request_fcs_time_, "  in=", input_fcs_time_, " -> ", (int)input_pull_result_);
			input_fcs_time_ = request_fcs_time_;
		}
		input_cv_.notify_all();
		
		if(input_pull_result_ == pull_result::stopped) break;
	}
	
	{
	//	std::lock_guard<std::shared_timed_mutex> in_lock(input_mutex_);
		this_node().unload_input_view_();
	}
}


void multiplex_node::async_loader::pre_stop() {
	Assert(this_node().graph().was_stopped());
	
	{
		std::lock_guard<std::mutex> req_lock(request_mutex_);
		stop_ = true;
	}
	request_cv_.notify_one();
}


void multiplex_node::async_loader::stop() {
	Assert(this_node().graph().was_stopped());
	Assert(thread_.joinable());
	
	thread_.join();
	stop_ = false;
}


void multiplex_node::async_loader::launch() {
	MpxDebug("mpx launch");
	thread_ = std::move(std::thread(
		std::bind(&multiplex_node::async_loader::thread_main_, this)
	));
	set_thread_name(thread_, "mpx writer");
}


void multiplex_node::async_loader::pre_pull(time_span span) {		
	if(input_fcs_time_ != this_node().capture_fcs_time_()) {
		request_cv_.notify_one();
	}
}


node::pull_result multiplex_node::async_loader::pull(time_span& span) {	
	auto func=[&]{
		
	MpxDebug("mpx pull ", span);
	
	std::shared_lock<std::shared_timed_mutex> in_lock(input_mutex_);
	
	if(input_pull_result_ == pull_result::success || input_pull_result_ == pull_result::end_of_stream)
		if(this_node().loaded_input_span_().includes(span)) return pull_result::success;

	time_unit new_fcs_time = this_node().capture_fcs_time_();
	time_span expected_input_span = this_node().expected_input_span_(new_fcs_time);
	if(! expected_input_span.includes(span)) return pull_result::transitory_failure;
	
	{
		std::lock_guard<std::mutex> req_lock(request_mutex_);
		request_fcs_time_ = new_fcs_time;
	}
	request_cv_.notify_one();
	
	input_cv_.wait(in_lock, [&] { return (input_pull_result_ == pull_result::stopped) || ((input_fcs_time_ != -1) && (input_fcs_time_ == request_fcs_time_)); });
	MpxDebug("inwaited req=", request_fcs_time_, "  in=", input_fcs_time_);
	
	if(input_pull_result_ == pull_result::stopped) return pull_result::stopped;
	
	Assert(input_pull_result_ != pull_result::undefined);
	
	time_span input_span = this_node().loaded_input_span_();
	expected_input_span = this_node().expected_input_span_(input_fcs_time_);

	if(input_pull_result_ == pull_result::success || input_pull_result_ == pull_result::end_of_stream) {
		MpxDebug("expected: ", expected_input_span, "   input: ", input_span);
		//Assert(expected_input_span.start_time() == input_span.start_time());
		time_span requested_span = span;
		span = span_intersection(input_span, span);
		
		if(input_span.includes(requested_span)) return pull_result::success;
		else return pull_result::end_of_stream;
	} else {
		span.set_end_time(span.start_time());
		return input_pull_result_;
	}
	
	};auto res=func();MpxDebug("mpx pull ", span, " -> ", (int)res);return res;
}



node_frame_window_view multiplex_node::async_loader::begin_read(time_span span) {	
	input_mutex_.lock_shared();
	
	timed_frame_array_view input_view = this_node().loaded_input_view_();
	if(input_view.is_null()) return node_frame_window_view::null();
	else if(! input_view.span().includes(span)) return node_frame_window_view::null();
	else return input_view.time_section(span);
}


void multiplex_node::async_loader::end_read(time_unit duration) {
	input_mutex_.unlock_shared();
}



}}
