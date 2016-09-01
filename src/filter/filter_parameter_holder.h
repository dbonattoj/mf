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

#ifndef MF_FLOW_FILTER_PARAMETER_HOLDER_H_
#define MF_FLOW_FILTER_PARAMETER_HOLDER_H_

#include "../flow/parameter/node_parameter_holder.h"
#include <utility>

namespace mf { namespace flow {

/// Holder for value of filter parameter.
template<typename Value>
class filter_parameter_holder : public node_parameter_holder {
public:
	using value_type = Value;

private:
	value_type value_;
	
public:
	template<typename... Args>
	explicit filter_parameter_holder(Args&&... args) :
		value_(std::forward<Args>(args)...) { }
	
	~filter_parameter_holder() override = default;
	
	filter_parameter_holder* clone() const override {
		return new filter_parameter_holder(value_);
	}
	
	const value_type& value() const { return value_; }
	value_type& value() { return value_; }
};

}}

#include "filter_parameter_holder.tcc"

#endif
