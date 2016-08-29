#ifndef MF_FLOW_FILTER_PARAMETER_VALUE_H_
#define MF_FLOW_FILTER_PARAMETER_VALUE_H_

#include <memory>

namespace mf { namespace flow {

class filter_parameter_value {
private:
	class holder_base;
	template<typename Value> class holder;

	std::unique_ptr<holder> holder_;

public:
	template<typename Value> explicit filter_parameter_value(const Value&);
	filter_parameter_value(const filter_parameter_value&);
	filter_parameter_value(filter_parameter_value&&);
	~filter_parameter_value();
	
	filter_parameter_value& operator=(const filter_parameter_value&);
	filter_parameter_value& operator=(filter_parameter_value&&);
	
	template<typename Value> bool has_type() const;
	template<typename Value> const Value& value() const;
	template<typename Value> Value& value();
};

}}

#include "filter_parameter_value.tcc"

#endif
