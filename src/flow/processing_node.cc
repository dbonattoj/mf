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
#include "processing_node_job.h"
#include "multiplex_node.h"
#include "graph.h"

namespace mf { namespace flow {


std::size_t processing_node_output::channels_count() const noexcept {
	return this_node().output_channels_.size();
}


std::string processing_node_output::channel_name_at(std::ptrdiff_t i) const {
	return this_node().output_channels_.at(i)->name();
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
		
	for(auto&& chan : output_channels_)
		if(! chan->frame_format().is_defined())
			throw invalid_node_graph("processing_node did not define all output channel formats");
}


void processing_node::handler_pre_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	handler_->handler_pre_process(*this, job);
}


void processing_node::handler_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	std::cout << name() << " process....................." << std::endl;
	
	if(this_graph().has_diagnostic())
		this_graph().diagnostic().processing_node_job_started(*this, job.time());
	
	handler_->handler_process(*this, job);

	if(this_graph().has_diagnostic())
		this_graph().diagnostic().processing_node_job_finished(*this, job.time());
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


processing_node::processing_node(graph& gr, bool with_output) :
	base(gr)
{
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
	output_channels_.emplace_back(new processing_node_output_channel(*this, channel_index));
	return *output_channels_.back();
}


processing_node_output_channel& processing_node::output_channel_at(std::ptrdiff_t index) {
	return *output_channels_.at(index);
}


const processing_node_output_channel& processing_node::output_channel_at(std::ptrdiff_t index) const {
	return *output_channels_.at(index);
}


node_frame_format processing_node::output_frame_format_() const {
	node_frame_format frm;
	for(auto&& chan : output_channels_) {
		const ndarray_format& channel_frame_format = chan->frame_format();
		Assert(channel_frame_format.is_defined());
		frm.add_part(channel_frame_format);
	}
	return frm;
}


}}
