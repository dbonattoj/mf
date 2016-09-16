#ifndef MF_FLOW_REALTIME_GATE_NODE_H_
#define MF_FLOW_REALTIME_GATE_NODE_H_

#include "node_derived.h"

namespace mf { namespace flow {

class node_graph;
class realtime_gate_node;

class realtime_gate_node_output final : public node_output {
private:
	realtime_gate_node& this_node();
	const realtime_gate_node& this_node() const;

public:
	explicit realtime_gate_node_output(node& nd) : node_output(nd) { }
	
	std::size_t channels_count() const noexcept override;
	std::string channel_name_at(std::ptrdiff_t i) const override;
	node::pull_result pull(time_span& span, bool reconnect) override;
	node_frame_window_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


class realtime_gate_node : public node_derived<node_input, realtime_gate_node_output> {
	using base = node_derived<node_input, realtime_gate_node_output>;

	// gate: connect
	// animated 
	//  to
	// realtime

private:
	std::thread thread_;
	ndarray_opaque<0> retained_frame_;
	
	void thread_main_();
	void load_new_frame_();

	// constantly reads 1 frame on separate thread
	// any copies it into retained_frame_ (mutex-protected)
	// --> use two retained_frames_, and do buffer swap
	// frame index of new frame to load:
	//   calculated from pull clock times
	//   --> expected next clock time --> frame time to pull

public:
	explicit realtime_gate_node(node_graph&);

	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
	
	node_input& input();
	const node_input& input() const;
	realtime_gate_node_output& output();
	const realtime_gate_node_output& output() const;

	void launch() override;
	void stop() override;
	void pre_setup() override;
	void setup() override;
};


inline realtime_gate_node& realtime_gate_node_output::this_node() {
	return static_cast<realtime_gate_node&>(node_output::this_node());
}

inline const realtime_gate_node& realtime_gate_node_output::this_node() const {
	return static_cast<const realtime_gate_node&>(node_output::this_node());
}

}}

#endif
