#include "media_graph.h"
#include "media_node.h"
#include "media_sink_node.h"
#include "../debug.h"

#include <iostream>

namespace mf {


media_graph::media_graph() {
	MF_DEBUG("media_graph");
}


media_graph::~media_graph() {
	MF_DEBUG("media_graph stopping");
	sink_->stop_graph();
	MF_DEBUG("media_graph no more");
}


void media_graph::setup() {
	if(setup_) throw std::logic_error("media graph was already set up");
	if(sink_ == nullptr) throw std::logic_error("no sink node was added to media graph");
	sink_->setup_graph();
	setup_ = true;
}


time_unit media_graph::current_time() const {
	if(! setup_) throw std::logic_error("media graph not set up");
	return sink_->current_time();
}


void media_graph::run_until(time_unit last_frame) {
	if(! setup_) throw std::logic_error("media graph not set up");
	while(sink_->current_time() < last_frame && !sink_->reached_end()) {
		sink_->pull_next_frame();
	}

	MF_DEBUG("graph::reached end!");
}


void media_graph::run() {
	if(! setup_) throw std::logic_error("media graph not set up");
	while(!sink_->reached_end()) {
		sink_->pull_next_frame();
	}

	MF_DEBUG("graph::reached end!");

	sink_->stop_graph();
}

}
