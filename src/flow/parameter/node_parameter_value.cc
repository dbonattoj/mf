#include "node_parameter_value.h"

namespace mf { namespace flow {

node_parameter_value::node_parameter_value(const node_parameter_value& val) :
	holder_(std::move(val.holder_->clone())) { }


node_parameter_value::node_parameter_value(node_parameter_value&& val) :
	holder_(std::move(val.holder_)) { }


node_parameter_value& node_parameter_value::operator=(const node_parameter_value& val) {
	holder_ = std::move(val.holder_->clone());
	return *this;
}


node_parameter_value& node_parameter_value::operator=(node_parameter_value&& val) {
	holder_ = std::move(val.holder_);
	val.holder_.reset();
	return *this;
}


}}
