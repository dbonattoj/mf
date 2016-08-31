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
