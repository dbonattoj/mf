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

#include "node_parameter.h"
#include "node_parameter_value.h"

namespace mf { namespace flow {

node_parameter::node_parameter(parameter_id id) :
	id_(id) { }


bool node_parameter::is_deterministic() const {
	return value_function_;
}


bool node_parameter::is_dynamic() const {
	return ! value_function_;
}


void node_parameter::set_constant_value(const node_parameter_value& val) {
	set_value_function([val] { return val; });
}


void node_parameter::set_dynamic() {
	value_function_ = nullptr;
}


node_parameter_value node_parameter::deterministic_value(time_unit frame_time) const {
	Assert(is_deterministic());
	return value_function_(frame_time);
}


}}
