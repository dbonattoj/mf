#ifndef MF_FLOW_SINK_NODE_H_
#define MF_FLOW_SINK_NODE_H_

#include "node_base.h"

namespace mf { namespace flow {

class sink_node : public node_base {
public:
	template<std::size_t Dim, typename Elem> using input = node_base::input<Dim, Elem>;

private:
	bool reached_end_ = false;

	void frame_();

public:	
	template<std::size_t Dim, typename Elem> using input_type = input<Dim, Elem>;

	sink_node() = default;

	void setup_graph();
	
	void pull_next_frame();
	void seek(time_unit);
	
	bool reached_end() const noexcept { return reached_end_; }
};

}}

#endif
