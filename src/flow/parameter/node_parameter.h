#ifndef MF_FLOW_NODE_PARAMETER_H_
#define MF_FLOW_NODE_PARAMETER_H_

#include "node_parameter_value.h"
#include "../types.h"
#include <string>

namespace mf { namespace flow {

/// Information about parameter belonging to node.
/** The current value of the parameter is stored separately, using a \ref node_parameter_valuation in the \ref node. */
class node_parameter {
private:
	node_parameter_id id_;
	node_parameter_value initial_value_;
	std::string name_;
	
public:
	node_parameter(node_parameter_id id, const node_parameter_value& initial_value) :
		id_(id), initial_value_(initial_value) { }
	
	node_parameter_id id() const { return id_; }
	const node_parameter_value& initial_value() const { return initial_value_; }

	void set_name(const std::string& nm) { name_ = nm; }
	const std::string name() const { return name_; }
};

}}

#endif
