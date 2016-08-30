#ifndef MF_FLOW_NODE_PARAMETER_VALUATION_H_
#define MF_FLOW_NODE_PARAMETER_VALUATION_H_

#include <map>
#include "node_parameter.h"

namespace mf { namespace flow {

class node_parameter_valuation {
private:
	std::map<node_parameter_id, node_parameter_value> values_;

public:
	node_parameter_valuation() = default;
	node_parameter_valuation(const node_parameter_valuation&) = default;
	node_parameter_valuation(node_parameter_valuation&&) = default;

	node_parameter_valuation& operator=(const node_parameter_valuation&) = default;
	node_parameter_valuation& operator=(node_parameter_valuation&&) = default;
	
	bool contains_parameter(node_parameter_id) const;
	const node_parameter_value& operator()(node_parameter_id) const;
	node_parameter_value& operator()(node_parameter_id);

	void add(const node_parameter&);
	friend node_parameter_valuation combine(const node_parameter_valuation&, const node_parameter_valuation&);
};

}}

#endif
