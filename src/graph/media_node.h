#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include <thread>
#include "../common.h"
#include "media_node_base.h"

namespace mf {
	
template<std::size_t Dim, typename Elem> class media_node_input;
template<std::size_t Dim, typename Elem> class media_node_output;

class media_node : public media_node_base {
private:
	std::thread thread_;
	bool sequential_;
	
	void thread_main_();
	void pull_frame_();

protected:
	void stop_() override;

public:
	template<std::size_t Dim, typename Elem> using input_type = media_node_input<Dim, Elem>;
	template<std::size_t Dim, typename Elem> using output_type = media_node_output<Dim, Elem>;

	explicit media_node(time_unit prefetch = 0);
	~media_node();

	void pull(time_unit target_time) override;
};

}

#endif
