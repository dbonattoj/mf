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

#ifndef MF_FLOW_FILTER_PARAMETER_H_
#define MF_FLOW_FILTER_PARAMETER_H_

#include <functional>

namespace mf { namespace flow {
/*
template<typename Value>
class filter_parameter {
public:
	using value_type = Value;
	using compute_function_type = Value(time_unit);
	
private:
	parameter_kind kind_ = parameter_kind::undefined;
	std::function<compute_function_type> compute_function_;
	bool dynamic_ = false;

public:
	parameter_kind kind() const { return kind_; }

	void set_dynamic();
	void set_constant(const Value&);
	template<typename Function> void set_time_function(const Function&);
	void set_mirror(const filter_parameter&);
};


template<typename Value>
class filter_request_parameter : public filter_parameter<Value> {
	using base = filter_parameter<Value>;
	
public:
	using value_type = typename base::value_type;
};


template<typename Value>
class filter_input_parameter {
public:
	using value_type = Value;
	using parameter_type = filter_parameter<Value>;

private:
	parameter_type* referenced_parameter_;
};
*/

}}

#endif
