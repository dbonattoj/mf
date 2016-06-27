#ifndef MF_FLOW_NODE_STREAM_PROPERTIES_H_
#define MF_FLOW_NODE_STREAM_PROPERTIES_H_

#include "../common.h"
#include <stdexcept>

namespace mf {

class node_stream_properties {
public:
	enum policy_type { undefined, seekable, forward };

private:
	policy_type policy_ = undefined;
	time_unit duration_ = -1;

public:
	node_stream_properties() = default;

	explicit node_stream_properties(policy_type pol, time_unit dur = -1) :
	policy_(pol), duration_(dur) {
		if(pol == seekable && dur == -1) throw std::invalid_argument("duration of seekable stream must be defined");
	}
	
	node_stream_properties(const node_stream_properties&) = default;
	node_stream_properties& operator=(const node_stream_properties&) = default;

	policy_type policy() const noexcept { return policy_; }
	time_unit duration() const noexcept { return duration_; }

	bool duration_is_defined() const noexcept { return (duration_ != -1); }
	
	bool is_seekable() const noexcept { return (policy_ == seekable); }
};

}

#endif
