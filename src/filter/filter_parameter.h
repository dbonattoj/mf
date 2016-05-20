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

#include "../common.h"
#include <functional>
#include <stdexcept>

namespace mf { namespace flow {

template<typename Value>
class filter_parameter {
public:
	using value_type = Value;
	using function_type = Value(time_unit t);

private:
	std::function<function_type> function_;

	filter_parameter(const filter_parameter&) = delete;
	filter_parameter& operator=(const filter_parameter&) = delete;
	
public:
	filter_parameter() = default;
	
	explicit filter_parameter(const Value& constant_value) {
		set_constant(constant_value);
	}
		
	bool is_defined() const noexcept { return !! function_; }
	
	value_type get(time_unit t) const {
		if(! is_defined()) throw std::runtime_error("node parameter is undefined");
		else return function_(t);
	}
	
	void set_constant(const Value& constant_value) {
		function_ = [constant_value](time_unit i) { return constant_value; };
	}
	
	template<typename Function>
	void set_time_function(Function&& func) {
		function_ = func;
	}
	
	void set_mirror(const filter_parameter& param) {
		set_time_function([&param](time_unit t) { return param.get(t); });
	}
	
	void unset() {
		function_ = nullptr;
	}
};

}}

#endif
