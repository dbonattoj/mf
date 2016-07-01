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

#include "filter_node.h"
#include "../filter/filter.h"
#include "../filter/filter_job.h"

namespace mf { namespace flow {

filter_node::filter_node(graph& gr) : node(gr) { }

filter_node::~filter_node() { }


void filter_node::setup_filter_() {
	Assert(outputs().size() <= 1, "filter node must have at most one output");
	filter_->setup();
}


void filter_node::pre_process_filter_(filter_node_job& job) {
	filter_job fjob(job);
	filter_->pre_process(fjob);
}


void filter_node::process_filter_(filter_node_job& job) {
	filter_job fjob(job);
	filter_->process(fjob);
}


filter_node_job filter_node::begin_job_() {
	return filter_node_job(*this);
}


void filter_node::finish_job_(filter_node_job& job) {
	bool reached_end = false;
	time_unit t = job.time();

	if(stream_properties().duration_is_defined() && t == stream_properties().duration() - 1) reached_end = true;
	if(job.end_was_marked()) reached_end = true;
	
	while(job.has_inputs()) {
		const node_input& in = job.pop_input();
		if(t == in.end_time() - 1) reached_end = true;
	}
	
	if(reached_end) mark_end_();
}


node_input& filter_node::add_input(time_unit past_window, time_unit future_window) {
	return add_input_<node_input>(past_window, future_window);
}


node_output& filter_node::add_output(const frame_format& format) {
	return add_output_<filter_node_output>(format);
}


////////////////////////////////


node::pull_result filter_node_output::pull(time_span& span, bool reconnect) {
	Expects(span.duration() > 0);
	return this_node().output_pull_(span, reconnect);
	Ensures(this_node().current_time() >= span.start_time());
}


timed_frame_array_view filter_node_output::begin_read(time_unit duration) {
	Expects(duration > 0);
	return this_node().output_begin_read_(duration);
}


void filter_node_output::end_read(time_unit duration) {
	this_node().output_end_read_(duration);
}


////////////////////////////////


filter_node_job::filter_node_job(filter_node& nd) :
	time_(nd.current_time()),
	inputs_stack_(),
	inputs_slots_(nd.inputs().size(), nullptr)
{
	inputs_stack_.reserve(nd.inputs().size());
}


filter_node_job::~filter_node_job() {
	Expects(! has_output(), "filter node must detach and close output before destruction of filter_node_job");
	if(has_inputs()) cancel_inputs();
}

void filter_node_job::attach_output(const frame_view& output_view) {
	bool det = false;
	output_view_.reset(output_view);
}

	
void filter_node_job::detach_output() {
	bool det = true;
	output_view_.reset();
}

bool filter_node_job::push_input(node_input& in) {
	std::ptrdiff_t index = in.index();
	timed_frame_array_view vw = in.begin_read_frame();
	if(vw.is_null()) return false;
	inputs_stack_.emplace_back(&in, vw);
	inputs_slots_[index] = &inputs_stack_.back();
	return true;
}

node_input& filter_node_job::pop_input() {
	input_view_handle handle = inputs_stack_.back();
	inputs_stack_.pop_back();
	std::ptrdiff_t index = handle.first->index();
	inputs_slots_[index] = nullptr;
	handle.first->end_read_frame();
	return *handle.first;
}

void filter_node_job::cancel_inputs() {
	while(inputs_stack_.size() > 0) {
		inputs_stack_.back().first->cancel_read_frame();
		inputs_stack_.pop_back();
	}
	for(auto* slot : inputs_slots_) slot = nullptr;
}

const timed_frame_array_view& filter_node_job::input_view(std::ptrdiff_t index) {
	return inputs_slots_[index]->second;
}

bool filter_node_job::has_input_view(std::ptrdiff_t index) const noexcept {
	if(index < 0 || index >= inputs_slots_.size()) return false;
	else return (inputs_slots_[index] != nullptr);
}

const frame_view& filter_node_job::output_view() {
	return output_view_;
}



}}
