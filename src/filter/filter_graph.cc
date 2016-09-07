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

#include "filter_graph.h"

namespace mf { namespace flow {

filter_graph::~filter_graph() {
	if(! was_setup()) return;
	// filters hold weak reference to graph (and nodes): need to first delete filters and then graph
	// and need to stop graph (and async node threads) before deleting filters
	node_graph_->stop();
	filters_.clear();
	node_graph_.reset();
}


parameter_id filter_graph::new_parameter_id() {
	return ++parameter_id;
}


void filter_graph::setup() {
	Expects(! was_setup());
	node_graph_.reset(new graph);
	for(auto&& filt : filters_) filt->install(*node_graph_);
	node_graph_->setup();
}


time_unit filter_graph::current_time() const {
	Expects(was_setup());
	return node_graph_->current_time();
}


void filter_graph::run_until(time_unit last_frame) {
	Expects(was_setup());
	node_graph_->run_until(last_frame);
}


void filter_graph::run_for(time_unit duration) {
	Expects(was_setup());
	node_graph_->run_for(duration);
}


bool filter_graph::run() {
	Expects(was_setup());
	return node_graph_->run();
}


void filter_graph::seek(time_unit target_time) {
	Expects(was_setup());
	node_graph_->seek(target_time);
}


}}
