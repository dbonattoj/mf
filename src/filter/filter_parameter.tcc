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
#include "../flow/node.h"
#include "../flow/parameter/node_parameter_value.h"

namespace mf { namespace flow {

template<typename Value>
filter_parameter<Value>::filter_parameter(filter& filt) :
	filter_(filt)
{
	filter_.register_parameter(*this);
}

template<typename Value>
bool filter_parameter<Value>::is_deterministic() const {
	return value_function_ ? true : false;
}


template<typename Value>
bool filter_parameter<Value>::is_dynamic() const {
	return value_function_ ? false : true;
}



template<typename Value>
void filter_parameter<Value>::set_mirror(const filter_parameter& other_par) {
	value_function_ = [&other_par](time_unit t) -> Value { return other_par.deterministic_value(t); };
}


template<typename Value> template<typename Function>
void filter_parameter<Value>::set_value_function(Function&& func) {
	value_function_ = std::forward<Function&&>(func);
}


template<typename Value>
void filter_parameter<Value>::set_constant_value(const Value& val) {
	value_function_ = [val](time_unit) -> Value { return val; };
}


template<typename Value>
void filter_parameter<Value>::set_dynamic(const Value& initial_value) {
	value_function_ = nullptr;
	dynamic_initial_value_.reset(new Value(initial_value));
}


template<typename Value>
const Value& filter_parameter<Value>::dynamic_initial_value() const {
	Assert(is_dynamic());
	return *dynamic_initial_value_;
}


template<typename Value>
bool filter_parameter<Value>::was_installed() const {
	return (id_ != undefined_node_parameter_id);
}


template<typename Value>
void filter_parameter<Value>::install(filter_graph& gr, node& nd) {
	if(was_installed()) return;
	id_ = gr.new_node_parameter_id();
	if(is_dynamic()) {
		node_parameter& par = nd.add_parameter(id_, *dynamic_initial_value_);
		par.set_name(name_);
	}
}


template<typename Value>
Value filter_parameter<Value>::deterministic_value(time_unit t) const {
	Assert(is_deterministic());
	return value_function_(t);
}


///////////////


template<typename Value>
filter_extern_parameter<Value>::filter_extern_parameter(filter& filt, bool input, bool sent) :
	filter_(filt),
	linked_parameter_(nullptr),
	input_(input),
	sent_(sent)
{
	filter_.register_extern_parameter(*this);
}


template<typename Value>
void filter_extern_parameter<Value>::link(parameter_type& param) {
	// TODO verify that param.filter_ precedes this->filter_ in the filter graph
	linked_parameter_ = &param;
}


template<typename Value>
bool filter_extern_parameter<Value>::is_linked() const {
	return (linked_parameter_ != nullptr);
}


template<typename Value>
auto filter_extern_parameter<Value>::linked_parameter() const -> const parameter_type& {
	Assert(is_linked());
	return *linked_parameter_;
}


template<typename Value>
void filter_extern_parameter<Value>::install(filter_graph& gr, node& nd) {
	Assert(is_linked());
	if(! linked_parameter().was_installed()) linked_parameter_->install(gr, nd);
	if(linked_parameter().is_dynamic()) {
		node_parameter_id id = linked_parameter().id();
		if(is_input_parameter()) nd.add_input_parameter(id);
		if(is_sent_parameter()) nd.add_sent_parameter(id);
	}
}


}}
