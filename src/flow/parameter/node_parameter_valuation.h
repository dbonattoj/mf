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

#ifndef MF_FLOW_NODE_PARAMETER_VALUATION_H_
#define MF_FLOW_NODE_PARAMETER_VALUATION_H_

#include <map>
#include "node_parameter.h"
#include "node_parameter_value.h"

namespace mf { namespace flow {

class node_parameter_valuation {
private:
	std::map<parameter_id, node_parameter_value> values_;

public:
	node_parameter_valuation() = default;
	node_parameter_valuation(const node_parameter_valuation&) = default;
	node_parameter_valuation(node_parameter_valuation&&) = default;

	node_parameter_valuation& operator=(const node_parameter_valuation&) = default;
	node_parameter_valuation& operator=(node_parameter_valuation&&) = default;
	
	bool has(parameter_id) const;
	const node_parameter_value& operator()(parameter_id) const;
	void set(parameter_id, const node_parameter_value&);
	void set_all(const node_parameter_valuation&);
	void set_all(node_parameter_valuation&&);
	void erase(parameter_id);
	void clear();

	friend node_parameter_valuation combine(const node_parameter_valuation&, const node_parameter_valuation&);
};

}}

#endif
