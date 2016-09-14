#ifndef MF_FLOW_NODE_PARAMETER_RELAY_H_
#define MF_FLOW_NODE_PARAMETER_RELAY_H_

#include "node_parameter.h"
#include <map>
#include <functional>

namespace mf { namespace flow {

class node_parameter_value;

class node_parameter_relay {
public:
	using handler_function_type = std::function<void(const node_parameter_value&)>;

private:
	std::map<node_parameters_id, handler_function_type> handlers_;

public:
	void set_handler(node_parameter_id, const send_function_type&);
	bool has_handler(node_parameter_id) const;
	const handler_function_type& handler(node_parameter_id) const;

	void send_parameter(node_parameter_id, const node_parameter_value& new_value) const;
};

}}

#endif
