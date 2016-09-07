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

#ifndef MF_FLOW_NODE_PARAMETER_H_
#define MF_FLOW_NODE_PARAMETER_H_

#include <string>
#include <memory>
#include <functional>
#include <utility>

namespace mf { namespace flow {

using parameter_id = int;
static constexpr undefined_parameter_id = 0;

class node_parameter_value;

/// Node parameter with its current value.
class node_parameter {
public:
	using deterministic_value_function = node_parameter_value(time_unit);

private:
	parameter_id id_;
	std::function<deterministic_value_function> value_function_;

public:
	explicit node_parameter(parameter_id id);
	
	parameter_id id() const { return id_; }	
	bool is_deterministic() const;
	bool is_dynamic() const;

	template<typename Function> set_value_function(Function&& func)
		{ value_function_ = std::forward<Function>(func); }
	void set_constant_value(const node_parameter_value&);
	void set_dynamic();
	
	node_parameter_value deterministic_value(time_unit frame_time) const;
};


}}

#endif
