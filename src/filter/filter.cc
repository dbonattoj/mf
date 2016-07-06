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

namespace mf { namespace flow {


void filter::register_input(filter_input_base& in) {
	Expects(! was_installed());
	inputs_.push_back(&in);
}


void filter::register_output(filter_output_base& out) {
	Expects(! was_installed());
	outputs_.push_back(&out);
}


void filter::set_asynchonous(bool async) {
	Expects(! was_installed());
	asynchronous_ = async;
}


bool filter::is_asynchonous() const {
	return asynchronous_;
}


void filter::set_prefetch_duration(time_unit dur) {
	Expects(! was_installed());
	prefetch_duration_ = dur;
}


time_unit filter::prefetch_duration() const {
	return prefetch_duration_;
}


void filter::install(graph& gr) {
	Expects(! was_installed());
	Expects(outputs_.size() == 1, "non-sink filter must have exactly one output");
	if(asynchronous_) {
		async_node& nd = gr.add_node<async_node>();
		nd.set_prefetch_duration(prefetch_duration_);
		node_ = &nd;
	} else {
		sync_node& nd = gr.add_node<sync_node>();
		node_ = &nd;
	}
	for(filter_input_base* in : inputs_) in->install(*node_);
	outputs_.front()->install(*node_);
	Ensures(was_installed());
}


void sink_filter::install(graph& gr) {
	Expects(! was_installed());
	Expects(outputs_.size() == 0, "sink filter must have no outputs");
	Expects(! is_asynchonous(), "sink filter cannot be asynchonous");
	Expects(prefetch_duration() == 0, "sink filter cannot have prefetch");
	
	sink_node& nd = gr.add_sink<sink_node>();
	node_ = &nd;

	for(filter_input_base* in : inputs_) in->install(*node_);
	Ensures(was_installed());
}


source_filter::source_filter(bool seekable, time_unit stream_duration) {
	auto policy = seekable ? node_stream_properties::seekable : node_stream_properties::forward;
	node_stream_properties_ = node_stream_properties(policy, stream_duration);
}


void source_filter::install(graph& gr) {
	Expects(! was_installed());
	Expects(inputs_.size() == 0, "source filter must have no inputs");
	Expects(outputs_.size() == 1, "non-sink filter must have exactly one output");
	if(asynchronous_) {
		async_node& nd = gr.add_node<async_node>();
		nd.set_prefetch_duration(prefetch_duration_);
		node_ = &nd;
	} else {
		sync_node& nd = gr.add_node<sync_node>();
		node_ = &nd;
	}
	node_->define_source_stream_properties(node_stream_properties_);
	outputs_.front()->install(*node_);
	Ensures(was_installed());
}


time_unit filter::current_time() const {
	Expects(was_installed());
	return node_->current_time();
}


bool filter::reached_end() const {
	Expects(was_installed());
	return node_->reached_end();
}

}}
