#ifndef MF_FLOW_GATE_NODE_H_
#define MF_FLOW_GATE_NODE_H_

#include "../node_derived.h"
#include "../node_output.h"
#include "../node_input.h"

namespace mf { namespace flow {

class gate_node;

class gate_node_output final : public node_output {
private:
	gate_node& this_node();
	const gate_node& this_node() const;

public:
	explicit gate_node_output(node& nd) : node_output(nd) { }
	
	std::size_t channels_count() const override;
	std::string channel_name_at(std::ptrdiff_t i) const override;
	const node_frame_format& frame_format() const override;
	void pre_pull(const time_span&) override;
	node::pull_result pull(time_span&) override;
	node_frame_window_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


class gate_node_input final : public node_input {
public:
	using node_input::node_input;

	thread_index reader_thread_index() const final override { return 0; }
};



class gate_node : public node_derived<gate_node_input, gate_node_output> {
	using base = node_derived<gate_node_input, gate_node_output>;
	friend class gate_node_output;

public:
	gate_node(node_graph&, const stream_timing& output_timing);

	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
	
	input_type& input() { return input_at(0); }
	const input_type& input() const { return input_at(0); }
	output_type& output() { return output_at(0); }
	const output_type& output() const { return output_at(0); }
	
	const stream_timing& input_timing() const { return input().connected_node().output_timing(); }

	virtual void output_pre_pull_(const time_span&) = 0;
	virtual node::pull_result output_pull_(time_span& span) = 0;
	virtual node_frame_window_view output_begin_read_(time_unit duration) = 0;
	virtual void output_end_read_(time_unit duration) = 0;
};


inline gate_node& gate_node_output::this_node() {
	return static_cast<gate_node&>(node_output::this_node());
}

inline const gate_node& gate_node_output::this_node() const {
	return static_cast<const gate_node&>(node_output::this_node());
}


inline bool is_gate_node(const node& nd) {
	return (dynamic_cast<const gate_node*>(&nd) != nullptr);
}

}}

#endif
