/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
