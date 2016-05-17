#include "filter_node.h"
#include "../filter/filter.h"

namespace mf { namespace flow {

filter_node::filter_node(graph& gr) : node(gr) { }

filter_node::~filter_node() { }

void filter_node::setup_filter() {
	MF_EXPECTS(filter_ != nullptr);
	filter_->setup();
}

void filter_node::pre_process_filter(node_job& job) {
	MF_EXPECTS(filter_ != nullptr);
	filter_->pre_process(job);
}

void filter_node::process_filter(node_job& job) {
	MF_EXPECTS(filter_ != nullptr);
	filter_->process(job);
}


}}
