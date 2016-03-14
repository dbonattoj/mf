#ifndef MF_MEDIA_SEQUENTIAL_NODE_H_
#define MF_MEDIA_SEQUENTIAL_NODE_H_

#include "media_node.h"

namespace mf {

class media_sequential_node : public media_node {
public:
	void pull_frame() override;
};


class media_sink_node : public media_sequential_node { };


class media_source_node : public media_sequential_node { };

}

#endif