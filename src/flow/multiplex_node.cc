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

#include "multiplex_node.h"
#include "graph.h"
#include <utility>
#include <functional>

namespace mf { namespace flow {

multiplex_node::multiplex_node(graph& gr) : node(gr) {
	add_input_<node_input>();
}


multiplex_node::~multiplex_node() {
	Assert(! thread_.joinable());
}


time_span multiplex_node::expected_input_span_() const {
	time_unit successor_time = successor_node_->current_time();
	return time_span(
		std::max(successor_time - input_past_window_, time_unit(0)),
		std::min(successor_time + input_future_window_ + 1, stream_properties().duration())
	);	
}


time_span multiplex_node::current_input_span_() const {
	return input_channel_views_.front().span();
}


void multiplex_node::load_input_view_(time_unit t) {
	std::lock_guard<std::shared_timed_mutex> view_lock(input_view_mutex_);

	set_current_time_(t);
	
	if(successor_time_of_input_view_ != -1)
		for(std::ptrdiff_t i = 0; i < input().channels_count(); ++i) input().end_read_frame(i);
		
	pull_result result = input().pull();
	if(result == pull_result::success) {
		successor_time_of_input_view_ = t;
		
		for(std::ptrdiff_t i = 0; i < input().channels_count(); ++i)
			input_channel_views_[i].reset(input().begin_read_frame(i));
		
	} else if(result == pull_result::transitory_failure) {
		successor_time_of_input_view_ = -1;
	}
}


void multiplex_node::thread_main_() {
	while(! this_graph().was_stopped()) {
		time_unit successor_time = successor_node_->current_time();
		std::unique_lock<std::mutex> lock(successor_time_mutex_);
		successor_time_changed_cv_.wait(lock, [&] {
			if(stopped_) return true;
			successor_time = successor_node_->current_time();
			return (successor_time_of_input_view_ != successor_time);
		});
		lock.unlock();
		if(stopped_) break;
		
		load_input_view_(successor_time);
		input_view_updated_cv_.notify_all();
	}
}


time_unit multiplex_node::minimal_offset_to(const node& target_node) const {
	return 0; // TODO
}


time_unit multiplex_node::maximal_offset_to(const node& target_node) const {
	return 0; // TODO
}

	
void multiplex_node::launch() {
	stopped_ = false;
	thread_ = std::move(std::thread(
		std::bind(&multiplex_node::thread_main_, this)
	));
}


void multiplex_node::stop() {
	Assert(this_graph().was_stopped());
	Assert(thread_.joinable());
	
	{
		std::lock_guard<std::mutex> lock1(successor_time_mutex_);
		std::lock_guard<std::shared_timed_mutex> lock2(input_view_mutex_);
		stopped_ = true;
	}
	successor_time_changed_cv_.notify_one();
	input_view_updated_cv_.notify_all();
	
	thread_.join();
}


void multiplex_node::pre_setup() {
	successor_node_ = &first_successor();

	input_past_window_ = input_future_window_ = 0;
	for(auto&& out : outputs()) {
		const node_input& in = out->connected_input();
		time_unit min_offset = in.this_node().minimal_offset_to(*successor_node_) - in.past_window_duration();
		time_unit max_offset = in.this_node().maximal_offset_to(*successor_node_) + in.future_window_duration();
		Assert(min_offset <= 0);
		Assert(max_offset >= 0);
		input_past_window_ = std::max(input_past_window_, -min_offset);
		input_future_window_ = std::max(input_future_window_, max_offset);		
	}
	input().set_past_window(input_past_window_);
	input().set_future_window(input_future_window_);
}


void multiplex_node::setup() {
	Assert(stream_properties().is_seekable());

	input_channel_views_.resize(input().channels_count());
}

	
node_input& multiplex_node::input() {
	Assert(inputs().size() == 1);
	return *inputs().front();
}


multiplex_node_output& multiplex_node::add_output() {
	return node::add_output_<multiplex_node_output>();
}


multiplex_node_output::multiplex_node_output(node& nd, std::ptrdiff_t index, std::ptrdiff_t input_channel_index) :
	node_output(nd, index),
	input_channel_index_(input_channel_index),
	input_view_shared_lock_(this_node().input_view_mutex_, std::defer_lock) { }


node::pull_result multiplex_node_output::pull(time_span& span, bool reconnect) {
	time_unit end_time = this_node().end_time();
	span = time_span(span.start_time(), std::min(span.end_time(), end_time));
	
	time_span expected_input_span = this_node().expected_input_span_();
	if(! expected_input_span.includes(span)) {
		MF_DEBUG_T("multiplex", "tfail1");
		return node::pull_result::transitory_failure;
	}
	
	std::shared_lock<std::shared_timed_mutex> lock(this_node().input_view_mutex_);
	
	while(this_node().successor_time_of_input_view_ != this_node().successor_node_->current_time()) {
		this_node().successor_time_changed_cv_.notify_one();
		if(this_node().stopped_) return node::pull_result::stopped;
		this_node().input_view_updated_cv_.wait(lock);
	}
	if(this_node().stopped_) return node::pull_result::stopped;
		
	time_span input_span = this_node().current_input_span_().span();
	
	if(input_span.includes(span)) {
		MF_DEBUG_T("multiplex", "success");
		return node::pull_result::success;
	} else {
		MF_DEBUG_T("multiplex", "tfail2");
		return node::pull_result::transitory_failure;
	}
}


std::size_t multiplex_node_output::channels_count() const noexcept {
	// multiplex node outputs always have 1 channel.
	// (but different outputs can be created for the different input channels)
	return 1;
}


timed_frame_array_view multiplex_node_output::begin_read(time_unit duration, std::ptrdiff_t channel_index) {
	Expects(channel_index == 0);
	
	Assert(! input_view_shared_lock_);
	input_view_shared_lock_.lock();
	
	timed_frame_array_view& input_view = this_node().input_channel_views_.at(input_channel_index_);
	
	const time_span& pulled_span = connected_input().pulled_span();
	Assert(duration == pulled_span.duration());
	
	Assert(! input_view.is_null());
	Assert(input_view.span().includes(pulled_span)); // ???
	
	std::ptrdiff_t start_index = input_view.time_index(pulled_span.start_time());
	std::ptrdiff_t end_index = start_index + duration;
	return input_view(start_index, end_index);
}


void multiplex_node_output::end_read(time_unit duration, std::ptrdiff_t channel_index) {
	Expects(channel_index == 0);
	Assert(input_view_shared_lock_);
	input_view_shared_lock_.unlock();
}



}}
