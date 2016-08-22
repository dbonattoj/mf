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

#include "filter.h"
#include "../flow/graph.h"
#include "../flow/node.h"
#include "../flow/sink_node.h"
#include "../flow/sync_node.h"
#include "../flow/async_node.h"
#include <set>

namespace mf { namespace flow {
	
const std::string filter::default_filter_name = "filter";
const std::string filter::default_filter_input_name = "in";
const std::string filter::default_filter_output_name = "out";


void filter::handler_setup(processing_node& nd) {
	Assert(&nd == node_);
	this->setup();
	for(filter_output_base* out : outputs_)
		Assert(out->frame_shape_is_defined());
}


void filter::handler_pre_process(processing_node& nd, processing_node_job& job) {
	Assert(&nd == node_);
	filter_job fjob(job);
	this->pre_process(fjob);
}


void filter::handler_process(processing_node& nd, processing_node_job& job) {
	Assert(&nd == node_);
	filter_job fjob(job);
	this->process(fjob);
}


void filter::register_input(filter_input_base& in) {
	Assert(! was_installed());
	inputs_.push_back(&in);
}


void filter::register_output(filter_output_base& out) {
	Assert(! was_installed());
	outputs_.push_back(&out);
}


void filter::set_asynchonous(bool async) {
	Assert(! was_installed());
	asynchronous_ = async;
}


bool filter::is_asynchonous() const {
	return asynchronous_;
}


void filter::set_prefetch_duration(time_unit dur) {
	Assert(! was_installed());
	prefetch_duration_ = dur;
}


time_unit filter::prefetch_duration() const {
	return prefetch_duration_;
}

bool filter::need_multiplex_node_() const {
	// multiplex node is needed if there are multiple output edges
	bool one_output_edge = false;
	for(auto&& out : outputs_) {
		std::size_t edges_count = out->edges_count();
		if(one_output_edge && edges_count >= 1) return true;
		else if(!one_output_edge && edges_count == 1) one_output_edge = true;
		else if(edges_count > 1) return true;
	}
	return false;
}


void filter::install(graph& gr) {
	Assert(! was_installed());
	/*if(asynchronous_) {
		async_node& nd = gr.add_node<async_node>();
		nd.set_prefetch_duration(prefetch_duration_);
		node_ = &nd;
	} else {*/
		sync_node& nd = gr.add_node<sync_node>();
		node_ = &nd;
	//}
	node_->set_name(name_.empty() ? default_filter_name : name_);
	node_->set_handler(*this);

	for(filter_input_base* in : inputs_) in->install(*node_);

	if(need_multiplex_node_()) {
		multiplex_node_ = &nd.this_graph().add_node<multiplex_node>();
		multiplex_node_->input().connect(node_->output());
		for(filter_output_base* out : outputs_) out->install(*node_, *multiplex_node_);
	} else {
		for(filter_output_base* out : outputs_) out->install(*node_);
	}

	Assert(was_installed());
}


void sink_filter::install(graph& gr) {
	Assert(! was_installed());
	Assert(outputs_.size() == 0, "sink filter must have no outputs");
	Assert(! is_asynchonous(), "sink filter cannot be asynchonous");
	Assert(prefetch_duration() == 0, "sink filter cannot have prefetch");
	
	sink_node& nd = gr.add_sink<sink_node>();
	node_ = &nd;
	node_->set_name(name_.empty() ? "sink" : name_);
	node_->set_handler(*this);

	for(filter_input_base* in : inputs_) in->install(*node_);
	Assert(was_installed());
}


source_filter::source_filter(bool seekable, time_unit stream_duration) {
	auto policy = seekable ? node_stream_properties::seekable : node_stream_properties::forward;
	node_stream_properties_ = node_stream_properties(policy, stream_duration);
}


void source_filter::define_source_stream_properties(const node_stream_properties& prop) {
	node_stream_properties_ = prop;
	// TODO cleanup
}


const node_stream_properties& source_filter::stream_properties() const noexcept { return node_stream_properties_; }



void source_filter::install(graph& gr) {
	Assert(! was_installed());
	Assert(inputs_.size() == 0, "source filter must have no inputs");

	/*if(asynchronous_) {
		async_node& nd = gr.add_node<async_node>();
		nd.set_prefetch_duration(prefetch_duration_);
		node_ = &nd;
	} else {*/
		sync_node& nd = gr.add_node<sync_node>();
		node_ = &nd;
	//}
	node_->set_name(name_.empty() ? "source" : name_);
	node_->set_handler(*this);
	node_->define_source_stream_properties(node_stream_properties_);

	if(need_multiplex_node_()) {
		multiplex_node_ = &nd.this_graph().add_node<multiplex_node>();
		multiplex_node_->input().connect(node_->output());
		for(filter_output_base* out : outputs_) out->install(*node_, *multiplex_node_);
	} else {
		for(filter_output_base* out : outputs_) out->install(*node_);
	}

	Assert(was_installed());
}


time_unit filter::current_time() const {
	Assert(was_installed());
	return node_->current_time();
}


bool filter::reached_end() const {
	Assert(was_installed());
	return node_->reached_end();
}

}}
