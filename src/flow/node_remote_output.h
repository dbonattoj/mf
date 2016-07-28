#ifndef MF_FLOW_NODE_REMOTE_OUTPUT_H_
#define MF_FLOW_NODE_REMOTE_OUTPUT_H_

#include "node.h"

namespace mf { namespace flow {

class node_output;

/// Output port of another node, read interface for connected node.
class node_remote_output {
public:
	virtual node_output& this_output() noexcept = 0;
	virtual time_unit end_time() const noexcept = 0;
	
	virtual node::pull_result pull(time_span& span, bool reconnect) = 0;
	virtual timed_frame_array_view begin_read(time_unit duration) = 0;
	virtual void end_read(time_unit duration) = 0;
};

}}

#endif
