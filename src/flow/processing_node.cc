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


std::size_t processing_node_output::channels_count() const noexcept {
	return this_node().output_channels_count();
}


node::pull_result processing_node_output::pull(time_span& span, bool reconnect) {
	return this_node().output_pull_(span, reconnect);
}


timed_frame_array_view processing_node_output::begin_read(time_unit duration) {
	return this_node().output_begin_read_(duration);
}


void processing_node_output::end_read(time_unit duration) {
	return this_node().output_end_read_(duration);
}


///////////////


processing_node_input::processing_node_input(processing_node& nd, std::ptrdiff_t index) :
	node_input(nd), index_(index) {}


///////////////


void processing_node::verify_connections_validity_() const {
	if(outputs().size() > 1) throw invalid_node_graph("processing_node must have at most 1 output");
	
	if(has_output() && output_channels_count() > 1) {
		node& successor = output().connected_node();
		if(typeid(successor) != typeid(multiplex_node))
			throw invalid_node_graph("processing_node output with >1 channel must be connected to multiplex_node");
	}
}


void processing_node::handler_setup_() {
	Expects(handler_ != nullptr);
	handler_->handler_setup(*this);
	
	for(const output_channel_type& chan : output_channels_)
		if(! chan.format().is_defined()) throw invalid_node_graph("processing_node did not set output channel format");
}


void processing_node::handler_pre_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	handler_->handler_pre_process(*this, job);
}


void processing_node::handler_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	handler_->handler_process(*this, job);
}


processing_node_job processing_node::begin_job_() {
	return processing_node_job(*this);
}


void processing_node::finish_job_(processing_node_job& job) {
	bool reached_end = false;
	
	if(stream_properties().duration_is_defined()
		&& current_time() == stream_properties().duration() - 1) reached_end = true;
	else if(job.end_was_marked()) reached_end = true;
	
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		if(job.has_input_view(i)) {
			input_type& in = input_at(i);
			if(current_time() == in.end_time() - 1) reached_end = true;
			job.end_input(in);
		}
	}
		
	if(reached_end) mark_end_();
}


std::size_t processing_node::output_channels_count() const noexcept {
	return output_channels_.size();
}


processing_node::processing_node(graph& gr, bool with_output) : base(gr) {
	if(with_output) add_output_(*this);
}


processing_node::~processing_node() { }


void processing_node::set_handler(processing_node_handler& handler) {
	handler_ = &handler;
}


processing_node_input& processing_node::add_input() {
	std::ptrdiff_t index = inputs_count();
	return add_input_(*this, index);
}


processing_node_output_channel& processing_node::add_output_channel() {
	std::ptrdiff_t channel_index = output_channels_.size();
	output_channels_.emplace_back(*this, channel_index);
	return output_channels_.back();
}


processing_node_output_channel& processing_node::output_channel_at(std::ptrdiff_t index) {
	return output_channels_.at(index);
}


const processing_node_output_channel& processing_node::output_channel_at(std::ptrdiff_t index) const {
	return output_channels_.at(index);
}



///////////////


processing_node_job::processing_node_job(processing_node& nd) :
	node_(nd),
	input_handles_(nd.inputs_count()) { }


processing_node_job::~processing_node_job() {
	Expects(output_view_.is_null(), "processing_node_job must be detached prior to destruction");
	cancel_inputs();
}


bool processing_node_job::begin_input(processing_node_input& in) {
	std::ptrdiff_t index = in.index();
	timed_frame_array_view vw = in.begin_read_frame();
	if(vw.is_null()) return false;
	input_handles_.at(index) = input_view_handle(&in, vw);
	return true;
}


void processing_node_job::end_input(processing_node_input& in) {
	std::ptrdiff_t index = in.index();
	in.end_read_frame();	
	set_null_(input_handles_.at(index));
}


void processing_node_job::cancel_inputs() {
	for(input_view_handle& in : input_handles_) if(! is_null_(in)) {
		in.first->cancel_read_frame();	
		set_null_(in);
	}
}


void processing_node_job::attach_output_view(const frame_view& out_vw) {
	output_view_.reset(out_vw);
}


void processing_node_job::detach_output_view() {
	output_view_.reset();
}


bool processing_node_job::has_input_view(std::ptrdiff_t index) const noexcept {
	return ! is_null_(input_handles_.at(index));
}


const timed_frame_array_view& processing_node_job::input_view(std::ptrdiff_t index) const {
	Expects(has_input_view(index));
	return input_handles_.at(index).second;
}


bool processing_node_job::has_output_view() const noexcept {
	return ! output_view_.is_null();
}


const frame_view& processing_node_job::output_view() const {
	Expects(has_output_view());
	return output_view_;
}

}}
