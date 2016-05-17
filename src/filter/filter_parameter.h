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
