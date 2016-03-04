#ifndef MF_MEDIA_GRAPH_H_
#define MF_MEDIA_GRAPH_H_

#include "media_node.h"
#include <memory>

namespace mf {

class media_graph {
private:
	std::unique_ptr<media_node_base> sink_;
};

}

#endif
