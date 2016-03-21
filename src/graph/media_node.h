#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include <thread>
#include "../common.h"
#include "media_node_base.h"

namespace mf {

class media_node : public media_node_base {
private:
	std::thread thread_;
	bool sequential_;
	
	void thread_main_();
	void pull_frame_();

public:
	explicit media_node(time_unit prefetch = 0);
	~media_node();

	void pull(time_unit target_time) override;
};

}

#endif
