#include "node_parameter_value.h"

namespace mf { namespace flow {
	

node_parameter_value::node_parameter_value() :
	holder_() { }


node_parameter_value::node_parameter_value(const node_parameter_value& val) {
	if(val.holder_) holder_.reset(val.holder_->clone());
}


node_parameter_value::node_parameter_value(node_parameter_value&& val) :
	holder_(std::move(val.holder_)) { }
	

node_parameter_value& node_parameter_value::operator=(const node_parameter_value& val) {
	if(val.holder_) holder_.reset(val.holder_->clone());
	else holder_.reset();
	return *this;
}


node_parameter_value& node_parameter_value::operator=(node_parameter_value&& val) {
	holder_ = std::move(val.holder_);
	return *this;
}


}}
