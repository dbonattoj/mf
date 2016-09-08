#include "filter.h"
#include "filter_graph.h"
#include "../flow/node.h"
#include "../flow/node_parameter.h"
#include "../flow/node_parameter_value.h"

namespace mf { namespace flow {

template<typename Value>
filter_parameter<Value>::filter_parameter(filter& filt) :
	filter_(filt),
	id_(filter_.this_graph().new_parameter_id()) { }


template<typename Value>
bool filter_parameter<Value>::is_deterministic() const {
	return value_function_ ? true : false;
}


template<typename Value>
bool filter_parameter<Value>::is_dynamic() const {
	return value_function_ ? false : true;
}


template<typename Value> template<typename Function>
void filter_parameter<Value>::set_value_function(Function&& func) {
	value_function_ = std::forward<Function&&>(func);
}


template<typename Value>
void filter_parameter<Value>::set_constant_value(const Value& val) {
	value_function_ = [val](time_unit) -> Value { return val; }
}


template<typename Value>
void filter_parameter<Value>::set_dynamic() {
	value_function_ = nullptr;
}


template<typename Value>
void filter_parameter<Value>::install(node& nd) {
	if(is_dynamic()) nd.add_parameter(id_);
}


///////////////


template<typename Value>
filter_extern_parameter<Value>::filter_extern_parameter(filter& filt) :
	filter_(filt),
	linked_id_(undefined_parameter_id) { }


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
void filter_extern_parameter<Value>::install(node& nd) {
	Assert(is_linked());
	if(linked_parameter().is_dynamic() && readable_)
		nd.add_input_parameter(linked_parameter().id());
}


}}
