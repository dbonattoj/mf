#ifndef MF_MEDIA_SEQUENTIAL_NODE_H_
#define MF_MEDIA_SEQUENTIAL_NODE_H_

#include "media_node.h"

namespace mf {

/// Base class for media node that processes frames on demand.
/** The sequential media node processes frames on demand when pulled with pull_frames(). During this is recursively
 ** pulls in required frames from preceding nodes. */
class media_sequential_node : public media_node {
private:
	void pull_frame_();

public:
	media_sequential_node() : media_node(0) { }

	void pull(time_unit target_time) override;
};

}

#endif