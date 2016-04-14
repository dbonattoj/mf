#ifndef MF_FLOW_SINK_NODE_H_
#define MF_FLOW_SINK_NODE_H_

#include <thread>
#include "../common.h"
#include "node_base.h"

namespace mf { namespace flow {
	
template<std::size_t Dim, typename Elem> class node_input;

class sink_node : public node_base {
private:
	std::atomic<bool> reached_end_{false};

	void pull_frame_();
			
public:
	template<std::size_t Dim, typename Elem> using input_type = node_input<Dim, Elem>;

	void setup_graph();
	void stop_graph();

	void pull_next_frame();

	void seek(time_unit t);

	bool reached_end() const { return reached_end_; }

	explicit sink_node();
};

}}

#endif
