#ifndef MF_FLOW_FILTER_NODE_H_
#define MF_FLOW_FILTER_NODE_H_

#include "node.h"
#include "sync_node.h"
#include "async_node.h"

namespace mf { namespace flow {

class filter_node : public node {
private:
	bool asynchronous_ = false;
	union {
		sync_node sync_;
		async_node async_;
	};
};

}}

#endif
