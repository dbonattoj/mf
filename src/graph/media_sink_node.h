#ifndef MF_MEDIA_SINK_NODE_H_
#define MF_MEDIA_SINK_NODE_H_

#include "media_node.h"

namespace mf {

/// Media node that acts as sink.
/** Sink node has no outputs. Graph has exactly one sink. Sink node is sequential: pull_frames() is synchronous, and
 ** current_time() and reached_end() reflect state after last pull_frames() call. */
class media_sink_node : public media_node {
public:
	void setup_graph();
	void stop_graph();

	void pull_next_frame();	
	
	media_sink_node() : media_node(0) { }
};

}

#endif
