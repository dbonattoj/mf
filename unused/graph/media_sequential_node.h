#ifndef MF_MEDIA_SEQUENTIAL_NODE_H_
#define MF_MEDIA_SEQUENTIAL_NODE_H_

#include "media_node.h"

namespace mf {

class media_sequential_node : public media_node {
private:
	void pull_frame_();

public:
	void pull_frames(time_unit target_time) override;
};


class media_sink_node : public media_sequential_node { };


class media_source_node : public media_sequential_node { };

}

#endif