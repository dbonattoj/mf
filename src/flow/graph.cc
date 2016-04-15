#include "graph.h"
#include "node.h"
#include "sink_node.h"
#include "../debug.h"
#include <limits>

#include <iostream>

namespace mf { namespace flow {


graph::~graph() {
	if(running_) stop();
}


void graph::setup() {
	if(was_setup_) throw std::logic_error("media graph was already set up");
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
	for(const auto& nd : nodes_) nd->stop();
	running_ = false;
}


time_unit graph::current_time() const {
	if(! was_setup_) throw std::logic_error("graph not set up");
	return sink_->current_time();
}

void graph::run_until(time_unit last_frame) {
	if(! was_setup_) throw std::logic_error("graph not set up");
	
	if(! running_) launch();
	
	while(sink_->current_time() < last_frame && !sink_->reached_end()) {
		sink_->pull_next_frame();
	}

	MF_DEBUG("graph::reached end!");
}


void graph::run_for(time_unit duration) {
	run_until(current_time() + duration);
}


void graph::run() {
	if(! sink_->is_bounded()) throw std::logic_error("sink is not bounded");
	run_until(std::numeric_limits<time_unit>::max());
}


void graph::seek(time_unit target_time) {
	sink_->seek(target_time);
}



}}
