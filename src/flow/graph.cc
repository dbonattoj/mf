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
#include "../utility/stopwatch.h"

namespace mf { namespace flow {


void graph::pull_next_frame_() {
	stopwatch watch;
	watch.start();
	sink_->pull_next_frame();
	watch.stop();
	
	if(callback_function) callback_function(sink_->current_time());
}

graph::~graph() {
	if(running_) stop();
}


void graph::setup() {
	if(was_setup_) throw std::logic_error("graph was already set up");
	if(sink_ == nullptr) throw std::logic_error("no sink node was added to media graph");
	sink_->setup_graph();
	was_setup_ = true;
}


void graph::launch() {
	if(running_) throw std::logic_error("graph is already running");
	for(const auto& nd : nodes_) nd->launch();
	running_ = true;
}


void graph::stop() {
	if(! running_) throw std::logic_error("graph is not running");
	stop_event_.notify();
	for(const auto& nd : nodes_) nd->stop();
	stop_event_.reset();
	running_ = false;
}


time_unit graph::current_time() const {
	if(! was_setup_) throw std::logic_error("graph not set up");
	return sink_->current_time();
}

void graph::run_until(time_unit last_frame) {
	if(! was_setup_) throw std::logic_error("graph not set up");
	
	if(! running_) launch();
	
	while(sink_->current_time() < last_frame && !sink_->reached_end())
		pull_next_frame_();
}


void graph::run_for(time_unit duration) {
	run_until(current_time() + duration);
}


bool graph::run() {
	if(! was_setup_) throw std::logic_error("graph not set up");
	if(! sink_->is_bounded()) throw std::logic_error("sink is not bounded");
	if(! running_) launch();
	while(sink_->is_bounded() && !sink_->reached_end()) pull_next_frame_();
	return sink_->is_bounded();
}


void graph::seek(time_unit target_time) {
	if(! sink_->is_seekable()) throw std::logic_error("sink is not seekable");
	sink_->seek(target_time);
}



}}
