#ifndef MF_FLOW_SINK_NODE_H_
#define MF_FLOW_SINK_NODE_H_

#include "node.h"

namespace mf { namespace flow {

class sink_node : public node {
private:
	bool reached_end_ = false;
	
	void frame_();
	
public:	
	sink_node() = default;
	
	void setup_graph();
	
	void pull_next_frame();
	void seek(time_unit);
	
	bool reached_end() const noexcept { return reached_end_; }
};


}}

#endif
