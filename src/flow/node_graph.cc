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

#include "node_graph.h"
#include "node.h"
#include "processing/sink_node.h"

#include "diagnostic/node_graph_visualization.h"
#include <thread>

namespace mf { namespace flow {

using namespace std::chrono_literals;

node::pull_result node_graph::pull_next_frame_() {
	Assert(launched_);

			export_node_graph_visualization(*this, "gr.gv");

	node::pull_result res = sink_->pull_next_frame();	
	if(callback_function) callback_function(sink_->current_time());
	
	return res;
	
	//std::this_thread::sleep_for(100ms);
}

node_graph::~node_graph() {
	if(launched_) stop();
}


void node_graph::setup() {
	Assert(! was_setup_);
	Assert(sink_ != nullptr);

	sink_->setup_graph();
	for(const auto& nd : nodes_) Assert(nd->was_setup());
	
	was_setup_ = true;
	export_node_graph_visualization(*this, "gr.gv");
}


thread_index node_graph::new_thread_index() {
	return ++last_thread_index_;
}


thread_index node_graph::root_thread_index() const {
	return 0;
}


void node_graph::launch() {
	if(launched_) return;
	
	if(has_diagnostic()) diagnostic().launched(*this);
		
	was_stopped_ = false;
	launched_ = true;
	for(const auto& nd : nodes_) nd->launch();
}


void node_graph::stop() {
	if(! launched_) return;
	
//	if(has_diagnostic()) diagnostic().stopped(*this);
	
	was_stopped_.store(true);
	for(const auto& nd : nodes_) nd->pre_stop();
	for(const auto& nd : nodes_) nd->stop();
	launched_ = false;
}


time_unit node_graph::current_time() const {
	Assert(was_setup_);
	return sink_->current_time();
}

void node_graph::run_until(time_unit last_frame) {
	Assert(was_setup_);
		
	if(! launched_) launch();

	node::pull_result res = node::pull_result::success;
	while(sink_->current_time() < last_frame && res == node::pull_result::success) {
		res = pull_next_frame_();
	}
}


void node_graph::run_for(time_unit duration) {
	run_until(current_time() + duration);
}


void node_graph::run() {
	Assert(was_setup_);
	
	if(! launched_) launch();

	node::pull_result res = node::pull_result::success;
	while(res == node::pull_result::success) {
		res = pull_next_frame_();
	}
}


void node_graph::seek(time_unit target_time) {
	Assert(was_setup_);
	Assert(target_time >= 0);
	sink_->seek(target_time);
}



}}
