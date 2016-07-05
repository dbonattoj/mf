#ifndef MF_FLOW_FILTER_CAST_CONNECTOR_H_
#define MF_FLOW_FILTER_CAST_CONNECTOR_H_

namespace mf { namespace flow {

template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
class filter_cast_connector : public node_remote_output {	
private:
	node_output& node_output_;

public:
	explicit filter_cast_connector(node_output& out) : node_output_(out) { }

	node_output& this_output() noexcept override { return node_output_; }
	time_unit end_time() const noexcept override;
	node::pull_result pull(time_span& span, bool reconnect) override;
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};

}}

#include "filter_cast_connector.tcc"

#endif
