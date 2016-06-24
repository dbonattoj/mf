#ifndef MF_FLOW_NODE_STREAM_PROPERTIES_H_
#define MF_FLOW_NODE_STREAM_PROPERTIES_H_

#include "../common.h"
#include <stdexcept>

namespace mf {

class node_stream_properties {
public:
	enum kind_type { seekable, forward };

private:
	kind_type kind_;
	time_unit duration_;

public:
	explicit node_stream_properties(kind_type knd, time_unit dur = -1) :
	kind_(knd), duration_(dur) {
		if(knd == seekable && dur == -1) throw std::invalid_argument("duration of seekable stream must be defined");
	}
	
	kind_type kind() const noexcept { return kind_; }
	time_unit duration() const noexcept { return duration_; }
};

}

#endif
