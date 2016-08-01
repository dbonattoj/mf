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

#include "processing_node.h"
#include "multiplex_node.h"

namespace mf { namespace flow {

processing_node::processing_node(graph& gr) : node(gr) { }

processing_node::~processing_node() { }

void processing_node::verify_connections_validity_() const {
	/*
	if(outputs().size() == 0) throw invalid_node_graph("processing_node must have at least 1 output");
	
	if(outputs().size() > 1) {
		node_output& first_output = *output().front();
		if(! first_output.is_bundled() || ! first_output.is_bundle_chief())
			throw invalid_node_graph("processing_node first of multiple outputs must be bundle chief");
			
		node& successor = first_output.connected_node();
		if(typeid(successor) != typeid(multiplex_node))
			throw invalid_node_graph("processing_node multiple outputs must be connected to multiplex_node");
	
		for(auto it = outputs().cbegin() + 1; it != outputs().cend(); ++it) {
			if(! it->is_bundled() || &it->bundle_chief() != &first_output)
				throw invalid_node_graph("processing_node multiple outputs must be in same bundle");
			
			if(&it->connected_node() != &successor)
				throw invalid_node_graph("processing_node multiple outputs must all be connected to same node");
		}
	}
	*/
}

void processing_node::set_handler(processing_node_handler& handler) {
	Expects(handler_ == nullptr);
	handler_ = &handler;
}


void processing_node::handler_setup_() {
	Expects(handler_ != nullptr);
	Expects(outputs().size() <= 1, "processing_node must have at most one output");
	handler_->handler_setup();
}


void processing_node::handler_pre_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	handler_->handler_pre_process(job);
}


void processing_node::handler_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	handler_->handler_process(job);
}


processing_node_job processing_node::begin_job_() {
	return processing_node_job(*this);
}


void processing_node::finish_job_(processing_node_job& job) {
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


node_input& processing_node::add_input() {
	return add_input_<node_input>();
}


processing_node_output& processing_node::add_output() {
	Expects(outputs().size() == 0, "cannot add more than one output to processing_node");
	return add_output_<processing_node_output>();
}


////////////////////////////////


node::pull_result processing_node_output::pull(time_span& span, bool reconnect) {
	Expects(span.duration() > 0);
	return this_node().output_pull_(span, reconnect);
	Ensures(this_node().current_time() >= span.start_time());
}


timed_frame_array_view processing_node_output::begin_read(time_unit duration) {
	Expects(duration > 0);
	return this_node().output_begin_read_(duration);
}


void processing_node_output::end_read(time_unit duration) {
	this_node().output_end_read_(duration);
}


////////////////////////////////


processing_node_job::processing_node_job(processing_node& nd) :
	time_(nd.current_time()),
	inputs_stack_(),
	inputs_slots_(nd.inputs().size(), nullptr)
{
	inputs_stack_.reserve(nd.inputs().size());
}


processing_node_job::~processing_node_job() {
	Expects(! has_output(), "processing_node must detach and close output before destruction of processing_node_job");
	if(has_inputs()) cancel_inputs();
}

void processing_node_job::attach_output(const frame_view& output_view) {
	output_view_.reset(output_view);
}

	
void processing_node_job::detach_output() {
	output_view_.reset();
}

bool processing_node_job::push_input(node_input& in) {
	std::ptrdiff_t index = in.index();
	timed_frame_array_view vw = in.begin_read_frame();
	if(vw.is_null()) return false;
	inputs_stack_.emplace_back(&in, vw);
	inputs_slots_[index] = &inputs_stack_.back();
	return true;
}

node_input& processing_node_job::pop_input() {
	input_view_handle handle = inputs_stack_.back();
	inputs_stack_.pop_back();
	std::ptrdiff_t index = handle.first->index();
	inputs_slots_[index] = nullptr;
	handle.first->end_read_frame();
	return *handle.first;
}

void processing_node_job::cancel_inputs() {
	while(inputs_stack_.size() > 0) {
		inputs_stack_.back().first->cancel_read_frame();
		inputs_stack_.pop_back();
	}
	for(auto* slot : inputs_slots_) slot = nullptr;
}

const timed_frame_array_view& processing_node_job::input_view(std::ptrdiff_t index) {
	return inputs_slots_[index]->second;
}

bool processing_node_job::has_input_view(std::ptrdiff_t index) const noexcept {
	if(index < 0 || index >= inputs_slots_.size()) return false;
	else return (inputs_slots_[index] != nullptr);
}

const frame_view& processing_node_job::output_view() {
	return output_view_;
}



}}
