#include "node_parameter_valuation.h"

namespace mf { namespace flow {

bool node_parameter_valuation::contains_parameter(node_parameter_id id) const {
	return (values_.find(id) != values_.end());
}


const node_parameter_value& node_parameter_valuation::operator()(node_parameter_id id) const {
	return values_.at(id);
}


node_parameter_value& node_parameter_valuation::operator()(node_parameter_id id) {
	return values_.at(id);
}


void node_parameter_valuation::add(const node_parameter& par) {
	auto res = values_.emplace(par.id(), par.current_value());
}


node_parameter_valuation combine(const node_parameter_valuation& a, const node_parameter_valuation& b) {
	node_parameter_valuation ab = a;
	ab.values_.insert(b.values_.cbegin(), b.values_.cend());
	return ab;
}

}}
