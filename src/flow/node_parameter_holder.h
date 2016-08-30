#ifndef MF_FLOW_NODE_PARAMETER_HOLDER_H_
#define MF_FLOW_NODE_PARAMETER_HOLDER_H_

namespace mf { namespace flow {

/// Holder for node parameter value, abstract base class.
class node_parameter_holder {
private:
	node_parameter_holder(const node_parameter_holder&) = delete;
	node_parameter_holder& operator=(node_parameter_holder&) = delete;
	
public:
	virtual ~node_parameter_holder() = 0;
	virtual node_parameter_holder* clone() const = 0;
};

}}

#endif
