#if 0
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
	using frame_buffer_type = ndarray_opaque<0>;

	std::thread thread_;
	frame_buffer_type loaded_frame_;
	frame_buffer_type outputted_frame_;
	std::mutex mutex_;
	
	clock_time_point launch_clock_time_;
	
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

	time_unit minimal_offset_to(const node&) const override { throw 1; }
	time_unit maximal_offset_to(const node&) const override { throw 1; }
	
	input_type& input() { return input_at(0); }
	const input_type& input() const { return input_at(0); }
	output_type& output() { return output_at(0); }
	const output_type& output() const { return output_at(0); }

	void launch() override;
	void pre_stop() override;
	void stop() override;
	void pre_setup() override;
	void setup() override;
	
	void output_pre_pull_(const time_span&);
	node::pull_result output_pull_(time_span& span);
	node_frame_window_view output_begin_read_(time_unit duration);
	void output_end_read_(time_unit duration);
};


inline realtime_gate_node& realtime_gate_node_output::this_node() {
	return static_cast<realtime_gate_node&>(node_output::this_node());
}

inline const realtime_gate_node& realtime_gate_node_output::this_node() const {
	return static_cast<const realtime_gate_node&>(node_output::this_node());
}

}}

#endif
#endif
