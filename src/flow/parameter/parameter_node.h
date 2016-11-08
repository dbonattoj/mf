#ifndef MF_FLOW_PARAMETER_NODE_H_
#define MF_FLOW_PARAMETER_NODE_H_

#include "node_derived.h"
#include <mutex>

namespace mf { namespace flow {

class parameter_node_output : public node_output {
private:
	parameter_node& this_node();
	const parameter_node& this_node() const;
	
	ndarray_opaque<1> pulled_frame_;

public:
	explicit parameter_node_output(node&);
	
	std::size_t channels_count() const override { return 0; }
	std::string channel_name_at(std::ptrdiff_t i) const override { return ""; }
	const node_frame_format& frame_format() const override;
	void pre_pull(const time_span& span) override;
	node::pull_result pull(time_span& span) override;
	node_frame_window_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


class parameter_node : public node_derived<node_input, parameter_node_output> {
	using base = node_derived<node_input, parameter_node_output>;
	friend class parameter_node_output;
	
private:
	std::vector<node_parameter> parameters_;
	node_parameter_valuation parameter_valuation_;
	mutable std::mutex parameters_mutex_;
	
	node_frame_format frame_format_;
	std::unique_ptr<timed_ring> pulled_valuations_; 

	void update_parameter(parameter_id, const node_parameter_value&);
	void update_parameter(parameter_id, node_parameter_value&&);
	void update_parameters(const node_parameter_valuation&);
	void update_parameters(node_parameter_valuation&&);
	node_parameter_valuation current_parameter_valuation() const;

public:
	explicit parameter_node(node_graph&);

	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
	
	node_parameter& add_parameter(parameter_id, const node_parameter_value& initial_value);
	bool has_parameter(parameter_id) const;
	std::size_t parameters_count() const { return parameters_.size(); }
	const node_parameter& parameter_at(std::ptrdiff_t i) const { return parameters_.at(i); }

	parameter_node_output& add_output();
	
	void setup() override;
};


inline parameter_node& parameter_node_output::this_node() {
	return static_cast<parameter_node&>(node_output::this_node());
}

inline const parameter_node& parameter_node_output::this_node() const {
	return static_cast<const parameter_node&>(node_output::this_node());
}


inline bool is_parameter_node(const node& nd) {
	return (dynamic_cast<const parameter_node*>(&nd) != nullptr);
}



}}

#endif
