#ifndef MF_MEDIA_PARALLEL_NODE_H_
#define MF_MEDIA_PARALLEL_NODE_H_

#include "media_node.h"

namespace mf {

class media_parallel_node : public media_node {
public:
	void pull_frame() override;
};

}

#endif
