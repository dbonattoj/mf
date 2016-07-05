#include "filter_graph.h"

namespace mf { namespace flow {

filter_graph::~filter_graph() {
	// filters hold weak reference to graph (and nodes): need to first delete filters and then graph
	filters_.clear();
	graph_.reset();
}

void filter_graph::setup() {
	Expects(! was_setup());
	graph_.reset(new graph);
	for(filter* filt : filters_) filt->install(*graph_);
}


time_unit filter_graph::current_time() const {
	Expects(was_setup());
	return graph_->current_time();
}


void filter_graph::run_until(time_unit last_frame) {
	Expects(was_setup());
	graph_->run_until(last_frame);
}


void filter_graph::run_for(time_unit duration) {
	Expects(was_setup());
	graph_->run_for(duration);
}


bool filter_graph::run() {
	Expects(was_setup());
	return graph_->run();
}


void filter_graph::seek(time_unit target_time) {
	Expects(was_setup());
	graph_->seek(target_time);
}


}}
