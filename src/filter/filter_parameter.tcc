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

#include "filter.h"
#include "filter_graph.h"

namespace mf { namespace flow {

template<typename Value>
filter_parameter<Value>::filter_parameter(filter& filt, const std::string& name) :
	filter_(filt),
	name_(name)
{
	filter_.register_parameter(*this);
	set_undefined_();
}


template<typename Value>
void filter_parameter<Value>::set_undefined_() {
	kind_ = undefined;
	input_reference_ = false;
	sent_reference_ = false;
	deterministic_value_function_ = nullptr;
	dynamic_initial_value_.reset();
	referenced_parameter_ = nullptr;
}


template<typename Value>
void filter_parameter<Value>::set_constant_value(const Value& value) {
	set_undefined_();
	kind_ = deterministic;
	deterministic_value_function_ = [value](time_unit) -> Value { return value; };
}


template<typename Value>
void filter_parameter<Value>::set_value_function(deterministic_value_function func) {
	set_undefined_();
	kind_ = deterministic;
	deterministic_value_function_ = func;
}


template<typename Value>
void filter_parameter<Value>::set_dynamic(const Value& initial_value) {
	set_undefined_();
	kind_ = dynamic;
	dynamic_initial_value_.reset(new Value(initial_value));
}


template<typename Value>
void filter_parameter<Value>::set_reference(const filter_parameter& ref_param, bool input, bool sent) {
	Assert(ref_param.this_filter().precedes_strict(this_filter()));
	set_undefined_();
	kind_ = reference;
	referenced_parameter_ = &ref_param;
	input_reference_ = input;
	sent_reference_ = sent;
}

	
template<typename Value>
const Value& filter_parameter<Value>::dynamic_initial_value() const {
	Assert(kind() == dynamic);
	return *dynamic_initial_value_;
}


template<typename Value>
Value filter_parameter<Value>::deterministic_value(time_unit t) const {
	Assert(kind() == deterministic);
	return deterministic_value_function_(t);
}


template<typename Value>
auto filter_parameter<Value>::referenced_parameter() const -> const filter_parameter& {
	Assert(kind() == reference);
	return *referenced_parameter_;
}


template<typename Value>
bool filter_parameter<Value>::is_input_reference() const {
	Assert(kind() == reference);
	return input_reference_;
}


template<typename Value>
bool filter_parameter<Value>::is_sent_reference() const {
	Assert(kind() == reference);
	return sent_reference_;
}



template<typename Value>
void filter_parameter<Value>::install(filter_graph& fg, node& nd) {	
	if(kind() == dynamic) {
		id_ = fg.new_parameter_id();
		node_parameter& par = nd.add_parameter(id_, dynamic_initial_value());
		par.set_name(name_);
	} else if(kind() == reference) {
		if(referenced_parameter().kind() == dynamic) {
			parameter_id id = referenced_parameter().id();
			if(input_reference_) nd.add_input_parameter(id);
			if(sent_reference_) nd.add_sent_parameter(id);
		} else if(referenced_parameter().kind() == deterministic) {
			if(sent_reference_) throw invalid_filter_graph("sent filter parameter reference to deterministic invalid");
		} else {
			throw invalid_filter_graph("filter parameter reference must be to dynamic or deterministic parameter");
		}
	}
}


}}
