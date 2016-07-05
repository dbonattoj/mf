#ifndef MF_FLOW_FILTER_GRAPH_H_
#define MF_FLOW_FILTER_GRAPH_H_

#include "../flow/graph.h"
#include "filter.h"

namespace mf { namespace flow {

class filter_graph {
private:
	std::vector<std::unique_ptr<filter>> filters_;
};

}}

#endif
