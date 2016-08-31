#ifndef MF_FLOW_NODE_PARAMETER_H_
#define MF_FLOW_NODE_PARAMETER_H_

#include <string>
#include <memory>
#include "node_parameter_holder.h"

namespace mf { namespace flow {

using node_parameter_id = int;

/// Container of node parameter value.
class node_parameter_value {
private:	
	std::unique_ptr<node_parameter_holder> holder_;

public:
	node_parameter_value();
	node_parameter_value(const node_parameter_value&);
	node_parameter_value(node_parameter_value&&);
	
	node_parameter_value& operator=(const node_parameter_value&);
	node_parameter_value& operator=(node_parameter_value&&);
};


/// Node parameter with its current value.
class node_parameter {
private:
	node_parameter_id id_;
	node_parameter_value value_;

public:
	explicit node_parameter(node_parameter_id id);
	
	node_parameter_id id() const { return id_; }
	node_parameter_value& current_value() { return value_; }
	const node_parameter_value& current_value() const { return value_; }
};


}}

#endif
