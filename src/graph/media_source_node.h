#ifndef MF_MEDIA_SOURCE_NODE_H_
#define MF_MEDIA_SOURCE_NODE_H_

#include "media_node.h"

namespace mf {

class media_seekable_source_node : public media_node {
public:	
	media_seekable_source_node(time_unit duration) : media_node(0) {
		define_stream_duration(duration);
	}
};


class media_forward_source_node : public media_node {
protected:
	virtual bool reached_end() const = 0;

public:
	media_forward_source_node() : media_node(0) { }
};

}

#endif
