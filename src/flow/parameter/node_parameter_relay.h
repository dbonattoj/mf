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
	std::map<parameter_id, handler_function_type> handlers_;

public:
	void set_handler(parameter_id, const handler_function_type&);
	bool has_handler(parameter_id) const;
	const handler_function_type& handler(parameter_id) const;
	std::size_t handlers_count() const { return handlers_.size(); }

	void send_parameter(parameter_id, const node_parameter_value& new_value) const;
};

}}

#endif
