#ifndef MF_MEDIA_SINK_NODE_H_
#define MF_MEDIA_SINK_NODE_H_

#include <thread>
#include "../common.h"
#include "media_node_base.h"

namespace mf {
	
template<std::size_t Dim, typename Elem> class media_node_input;

class media_sink_node : public media_node_base {
private:
	std::atomic<bool> reached_end_;

	void pull_frame_();

protected:
	void stop_() override;
	void launch_() override;
			
public:
	template<std::size_t Dim, typename Elem> using input_type = media_node_input<Dim, Elem>;

	void setup_graph();
	void stop_graph();

	void pull_next_frame();

	void seek(time_unit t);

	bool reached_end() const { return reached_end_; }

	explicit media_sink_node();
};

}

#endif
