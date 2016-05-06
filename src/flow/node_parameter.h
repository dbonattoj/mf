#ifndef MF_FLOW_NODE_PARAMETER_H_
#define MF_FLOW_NODE_PARAMETER_H_

namespace mf { namespace flow {

template<typename Value>
class node_parameter {
private:
	using function_type = Value(time_unit t);
	using value_type = Value;
	
	std::function<function_type> function_;

public:
	node_parameter(const Value& constant_value = Value()) :
		function_([constant_value](time_unit t) { return constant_value }) { }
	
	node_parameter& operator=(const Value& constant_value) {
		function_ = [constant_value](time_unit t) { return constant_value };
	}
	
	value_type get(time_unit t) const {
		return function_(t);
	}
	
	template<typename Unary>
	void set_time_function(Unary func) {
		function_ = func;
	}
	
	void set_mirror(const node_parameter& param) {
		function_ = [&param](time_unit t) { return param.get(t); }
	}
};

}}

#endif
