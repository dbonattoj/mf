#include "filter.h"
#include "filter_graph.h"
#include "../flow/node.h"
#include "../flow/parameter/node_parameter_value.h"

namespace mf { namespace flow {

template<typename Value>
filter_parameter<Value>::filter_parameter(filter& filt) :
	filter_(filt) { }

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
	value_function_ = [val](time_unit) -> Value { return val; };
}


template<typename Value>
void filter_parameter<Value>::set_dynamic() {
	value_function_ = nullptr;
}


template<typename Value>
bool filter_parameter<Value>::was_installed() const {
	return (id_ != undefined_node_parameter_id);
}


template<typename Value>
void filter_parameter<Value>::install(filter_graph& gr, node& nd) {
	if(was_installed()) return;
	id_ = gr.new_node_parameter_id();
	if(is_dynamic()) nd.add_parameter(id_);
}


template<typename Value>
Value filter_parameter<Value>::deterministic_value(time_unit t) const {
	Assert(is_deterministic());
	return value_function_(t);
}


///////////////


template<typename Value>
filter_extern_parameter<Value>::filter_extern_parameter(filter& filt) :
	filter_(filt),
	linked_parameter_(nullptr) { }


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
	if(linked_parameter().is_dynamic() && readable_) {
		if(! linked_parameter().was_installed()) linked_parameter_->install(gr, nd);
		nd.add_input_parameter(linked_parameter().id());
	}
}


}}
