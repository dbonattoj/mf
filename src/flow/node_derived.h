#ifndef MF_FLOW_NODE_DERIVED_H_
#define MF_FLOW_NODE_DERIVED_H_

#include "node.h"
#include <utility>

namespace mf { namespace flow {

template<class Derived_input, typename Derived_output>
class node_derived : public node {
public:
	using input_type = Derived_input;
	using output_type = Derived_output;
	
public:
	using node::node;

	input_type& input_at(std::ptrdiff_t index) {
		return static_cast<input_type&>(node::input_at(index));
	}
	
	const input_type& input_at(std::ptrdiff_t index) const {
		return static_cast<const input_type&>(node::input_at(index));
	}

	output_type& output_at(std::ptrdiff_t index) {
		return static_cast<output_type&>(node::output_at(index));	
	}
	
	const output_type& output_at(std::ptrdiff_t index) const {
		return static_cast<const output_type&>(node::output_at(index));	
	}
};

}}

#endif
