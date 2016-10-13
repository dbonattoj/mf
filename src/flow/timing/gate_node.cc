#include "gate_node.h"

namespace mf { namespace flow {

std::size_t gate_node_output::channels_count() const noexcept {
	return this_node().input().channels_count();
}

std::string gate_node_output::channel_name_at(std::ptrdiff_t i) const {
	return this_node().input().channel_name_at(i);
}

const node_frame_format& gate_node_output::frame_format() const {
	return this_node().input().frame_format();
}

void gate_node_output::pre_pull(const time_span& span) {
	return this_node().output_pre_pull_(span);
}

node::pull_result gate_node_output::pull(time_span& span) {
	return this_node().output_pull_(span);
}

node_frame_window_view gate_node_output::begin_read(time_unit duration) {
	return this_node().output_begin_read_(duration);
}

void gate_node_output::end_read(time_unit duration) {
	return this_node().output_end_read_(duration);
}


///////////////


gate_node::gate_node(node_graph& gr, const stream_timing& output_timing) : base(gr) {
	add_input_(*this);
	add_output_(*this);
	define_output_timing(output_timing);
}


time_unit gate_node::minimal_offset_to(const node& nd) const {
	if(&nd == this) return 0;
	else throw std::logic_error("requested node time offset through gate");
}


time_unit gate_node::maximal_offset_to(const node& nd) const {
	if(&nd == this) return 0;
	else throw std::logic_error("requested node time offset through gate");	
}


}}
