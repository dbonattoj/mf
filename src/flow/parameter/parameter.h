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

#ifndef MF_FLOW_PARAMETER_H_
#define MF_FLOW_PARAMETER_H_

#include "parameter_value.h"
#include "../types.h"
#include <string>

namespace mf { namespace flow {

/// Information about parameter belonging to node.
/** The current value of the parameter is stored separately, using a \ref node_parameter_valuation in the \ref node. */
class parameter {
private:
	parameter_id id_;
	unique_parameter_value_ptr initial_value_;
	std::string name_;
	
public:
	template<typename Value>
	node_parameter(parameter_id id, const Value& initial_value) :
		id_(id),
		initial_value_(make_unique_parameter_value_ptr(initial_value)) { }
		
	parameter_id id() const { return id_; }
	const node_parameter_value& initial_value() const { return initial_value_; }

	void set_name(const std::string& nm) { name_ = nm; }
	const std::string name() const { return name_; }
};

}}

#endif
