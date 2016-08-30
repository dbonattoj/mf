#ifndef MF_FLOW_FILTER_PARAMETER_HOLDER_H_
#define MF_FLOW_FILTER_PARAMETER_HOLDER_H_

#include "../flow/node_parameter_holder.h"
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
