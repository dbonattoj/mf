#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include "common.h"
#include <vector>

namespace mf {

namespace detail {
	class media_node_input_base;
	class media_node_output_base;
}


class media_node {
protected:
	std::vector<detail::media_node_input_base*> inputs_;
	std::vector<detail::media_node_output_base*> outputs_;
	time_unit time_ = -1;
	
	void register_input_(detail::media_node_input_base&);	
	void register_output_(detail::media_node_output_base&);
		
	// implemented in concrete subclass
	virtual void process_() = 0;

public:
	virtual ~media_node() { }
		
	virtual void pull_frames(time_unit target_time) = 0;
	virtual bool reached_eof() const;
	
	time_unit current_time() const noexcept { return time_; }
};

}

#endif