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
#include "multiplex_node_loader.h"
#include "graph.h"
#include <utility>
#include <functional>

namespace mf { namespace flow {


multiplex_node::multiplex_node(graph& gr) : base(gr) {
	add_input_(*this);
	set_name("multiplex");
}


multiplex_node::~multiplex_node() { }


time_unit multiplex_node::input_view_time_() const {
	if(! loaded_input_view_.is_null()) return current_time();
	else return -1;
}


void multiplex_node::load_input_view_(time_unit t) {
	set_current_time_(t);
	
	unload_input_view_();
	
	pull_result result = input().pull();
	if(result == pull_result::success) {
		timed_frame_array_view vw = input().begin_read_frame();
		loaded_input_view_.reset(vw);
	}
}


void multiplex_node::unload_input_view_() {
	if(! loaded_input_view_.is_null()) {
		input().end_read_frame();
		loaded_input_view_.reset();
	}
}



time_span multiplex_node::expected_input_span_() const {
	time_unit successor_time = common_successor_node_->current_time();
	return time_span(
		std::max(successor_time - input_past_window_, time_unit(0)),
		std::min(successor_time + input_future_window_ + 1, stream_properties().duration())
	);	
}


time_unit multiplex_node::minimal_offset_to(const node& target_node) const {
	return 0; // TODO
}


time_unit multiplex_node::maximal_offset_to(const node& target_node) const {
	return 0; // TODO
}


bool multiplex_node::is_async() const {
	Assert(loader_);
	return loader_->is_async();
}

thread_index multiplex_node::loader_thread_index() const {
	Assert(loader_);
	return loader_->loader_thread_index();
}

	
void multiplex_node::launch() {
	Assert(loader_);
	loader_->launch();
}


void multiplex_node::stop() {
	Assert(loader_);
	loader_->stop();
}


void multiplex_node::pre_setup() {
	common_successor_node_ = &first_successor();

	input_past_window_ = input_future_window_ = 0;
	for(auto&& out : outputs()) {
		const node_input& in = out->connected_input();
		time_unit min_offset = in.this_node().minimal_offset_to(*common_successor_node_) - in.past_window_duration();
		time_unit max_offset = in.this_node().maximal_offset_to(*common_successor_node_) + in.future_window_duration();
		Assert(min_offset <= 0);
		Assert(max_offset >= 0);
		input_past_window_ = std::max(input_past_window_, -min_offset);
		input_future_window_ = std::max(input_future_window_, max_offset);		
	}
	input().set_past_window(input_past_window_);
	input().set_future_window(input_future_window_);
}


void multiplex_node::setup() {
	loader_.reset(new async_loader(*this));
}


multiplex_node_output& multiplex_node::add_output(std::ptrdiff_t input_channel_index) {
	return add_output_(*this, input_channel_index);
}


multiplex_node_output::multiplex_node_output(node& nd, std::ptrdiff_t input_channel_index) :
	node_output(nd),
	input_channel_index_(input_channel_index) { }


std::size_t multiplex_node_output::channels_count() const noexcept {
	// multiplex node outputs always have 1 channel.
	// (but different outputs can be created for the different input channels)
	return 1;
}


node::pull_result multiplex_node_output::pull(time_span& span, bool reconnect) {
	Assert(this_node().loader_);
	return this_node().loader_->pull(span);
}


timed_frame_array_view multiplex_node_output::begin_read(time_unit duration) {	
	Assert(this_node().loader_);

	time_span pulled_span = connected_input().pulled_span();
	Assert(duration == pulled_span.duration()); // can duration be smaller? (node interface contract)

	timed_frame_array_view vw = this_node().loader_->begin_read(pulled_span);
	Assert(! vw.is_null());
	Assert(vw.span().includes(pulled_span), "multiplex input view span does not include span to read");

	return extract_part(vw, input_channel_index_);
}


void multiplex_node_output::end_read(time_unit duration) {
	Assert(this_node().loader_);
	this_node().loader_->end_read(duration);
}



}}
