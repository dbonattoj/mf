#include "graph.h"
#include "node.h"
#include "sink_node.h"
#include "../debug.h"

#include <iostream>

namespace mf { namespace flow {


graph::graph() {
}


graph::~graph() {
	if(running_) stop();
}


void graph::setup() {
	if(setup_) throw std::logic_error("media graph was already set up");
	if(sink_ == nullptr) throw std::logic_error("no sink node was added to media graph");
	sink_->setup_graph();
	was_setup_ = true;
}


void graph::launch() {
	if(running_) throw std::logic_error("graph is already running");
	for(const auto& nd : nodes_) nd->launch();
}


void graph::stop() {
	if(! running_) throw std::logic_error("graph is not running");
	for(const auto& nd : nodes_) nd->stop();
}


time_unit graph::current_time() const {
	if(! setup_) throw std::logic_error("graph not set up");
	return sink_->current_time();
}

void graph::run_until(time_unit last_frame) {
	if(! setup_) throw std::logic_error("graph not set up");
	
	if(! running_) launch();
	
	while(sink_->current_time() < last_frame && !sink_->reached_end()) {
		sink_->pull_next_frame();
	}

	MF_DEBUG("graph::reached end!");
}

void run_for(time_unit durgation) {
	
}


void graph::run() {
	if(! setup_) throw std::logic_error("media graph not set up");
	while(!sink_->reached_end()) {
		sink_->pull_next_frame();
	}

	MF_DEBUG("graph::reached end!");
}


void seek(time_unit target_time) {
	
}



}}
