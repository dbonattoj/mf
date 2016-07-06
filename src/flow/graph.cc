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

#include "graph.h"
#include "node.h"
#include "sink_node.h"

namespace mf { namespace flow {


void graph::pull_next_frame_() {
	Expects(launched_);
	sink_->pull_next_frame();	
	if(callback_function) callback_function(sink_->current_time());
}

graph::~graph() {
	if(launched_) stop();
}


void graph::setup() {
	Expects(! was_setup_);
	Expects(sink_ != nullptr);

	sink_->setup_graph();
	was_setup_ = true;
}


void graph::launch() {
	if(launched_) return;
		
	was_stopped_ = false;
	launched_ = true;
	for(const auto& nd : nodes_) nd->launch();
}


void graph::stop() {
	if(! launched_) return;
	
	//MF_DEBUG_BACKTRACE("graph::stop");	
	was_stopped_.store(true);
	for(const auto& nd : nodes_) nd->pre_stop();
	for(const auto& nd : nodes_) nd->stop();
	launched_ = false;
}


time_unit graph::current_time() const {
	Expects(was_setup_);
	return sink_->current_time();
}

void graph::run_until(time_unit last_frame) {
	Expects(was_setup_);

	//MF_DEBUG_BACKTRACE("graph::run_until");
		
	if(! launched_) launch();
	
	while(sink_->current_time() < last_frame && !sink_->reached_end())
		pull_next_frame_();
}


void graph::run_for(time_unit duration) {
	run_until(current_time() + duration);
}


bool graph::run() {
	Expects(was_setup_);
	Expects(sink_->is_bounded());

	//MF_DEBUG_BACKTRACE("graph::run");

	if(! launched_) launch();

	while(sink_->is_bounded() && !sink_->reached_end()) pull_next_frame_();
	return sink_->is_bounded();
}


void graph::seek(time_unit target_time) {
	Expects(was_setup_);
	Expects(sink_->stream_properties().is_seekable());
	sink_->seek(target_time);
}



}}
