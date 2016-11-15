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

#ifndef MF_FLOW_PARAMETER_VALUE_H_
#define MF_FLOW_PARAMETER_VALUE_H_

#include <utility>
#include <memory>
#include <type_traits>

namespace mf { namespace flow {

class parameter_value_base {
public:
	virtual ~parameter_value_base() = default;
	virtual parameter_value_base* clone() const = 0;
};


template<typename Value>
class parameter_value {
public:
	using value_type = Value;

private:
	value_type value_;

public:
	parameter_value() = default;
	parameter_value(const parameter_value&) = default;
	parameter_value(const Value& val) : value_(val) { }
	parameter_value(Value&& val) : value_(std::move(val)) { }
	~parameter_value() override = default;
	
	parameter_value& operator=(const parameter_value& val) { value_ = val; return *this; }
	parameter_value& operator=(parameter_value&& val) { value_ = std::move(val); return *this; }
	
	const value_type& value() const { return value_; }
	
	parameter_value* clone() const override { return new parameter_value(value_); }
};


using shared_parameter_value_ptr = std::shared_ptr<const parameter_value_base>;
using unique_parameter_value_ptr = std::unique_ptr<parameter_value_base>;

template<typename Value>
shared_parameter_value_ptr make_shared_parameter_value_ptr(Value&& val) {
	using value_type = std::decay_t<Value>;
	using parameter_value_type = parameter_value<value_type>;
	return std::make_shared<const parameter_value_type>(std::forward<Value>(val));
}

template<typename Ptr>
shared_parameter_value_ptr clone_shared_parameter_value_ptr(const Ptr& ptr) {
	parameter_value_base* clone = ptr->clone();
	return std::shared_ptr<const parameter_value_base>(clone);
}

template<typename Value>
unique_parameter_value_ptr make_unique_parameter_value_ptr(Value&& val) {
	using value_type = std::decay_t<Value>;
	using parameter_value_type = parameter_value<value_type>;
	return std::make_unique<parameter_value_type>(std::forward<Value>(val));
}

template<typename Ptr>
unique_parameter_value_ptr clone_unique_parameter_value_ptr(const Ptr& ptr) {
	parameter_value_base* clone = ptr->clone();
	return std::unique_ptr<parameter_value_base>(clone);
}


}}

#endif

