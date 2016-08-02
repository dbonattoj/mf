#ifndef MF_FLOW_NODE_REMOTE_OUTPUT_H_
#define MF_FLOW_NODE_REMOTE_OUTPUT_H_

#include "node.h"

namespace mf { namespace flow {

class node_remote_output {
public:
	virtual void input_has_connected(node_input&) = 0;
	virtual void input_has_disconnected() = 0;

	virtual time_unit end_time() const noexcept = 0;
	
	virtual std::size_t channels_count() const noexcept = 0;
	virtual node::pull_result pull(time_span& span, bool reconnect) = 0;
	virtual timed_frame_array_view begin_read(time_unit duration, std::ptrdiff_t channel_index) = 0;
	virtual void end_read(time_unit duration, std::ptrdiff_t channel_index) = 0;
};

}}

#endif
