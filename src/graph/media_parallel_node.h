#ifndef MF_MEDIA_PARALLEL_NODE_H_
#define MF_MEDIA_PARALLEL_NODE_H_

#include <thread>
#include "media_node.h"

namespace mf {

class media_parallel_node : public media_node {
private:
	std::thread thread_;
	
	void thread_main_();

public:
	explicit media_parallel_node(time_unit prefetch = 1);
	~media_parallel_node();

	void pull(time_unit target_time) override;
};

}

#endif
