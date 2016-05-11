#ifndef MF_FLOW_FILTER_NODE_H_
#define MF_FLOW_FILTER_NODE_H_

#include "node.h"

namespace mf { namespace flow {

class filter;
class graph;
class node_job;

/// Node which delegates concrete frame processing to associated \ref filter object.
class filter_node : public node {
private:
	filter* filter_ = nullptr;

protected:
	void setup_filter();
	void pre_process_filter(node_job&);
	void process_filter(node_job&);

public:
	filter_node(graph& gr) : node(gr) { }
	
	void set_filter(filter& filt) { filter_ = filt; }
	
	filter& this_filter() { return *filter_; }
	const filter& this_filter() const { return *filter_; }
};

}}

#endif
