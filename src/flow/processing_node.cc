#include "processing_node.h"

namespace mf { namespace flow {


std::size_t processing_node_output::channel_count() const noexcept {
	return this_node().output_channel_count();
}


node::pull_result processing_node_output::pull(time_span& span, bool reconnect) {
	return this_node().output_pull_(span, reconnect);
}


timed_frame_array_view processing_node_output::begin_read(time_unit duration, std::ptrdiff_t channel_index) {
	return this_node().output_begin_read_(duration, channel_index);
}


void processing_node_output::end_read(time_unit duration, std::ptrdiff_t channel_index) {
	return this_node().output_end_read_(duration, channel_index);
}


///////////////


void processing_node::verify_connections_validity_() const {
	if(outputs().size() > 1) throw invalid_node_graph("processing_node must have at most 1 output");
	
	if(has_output() && output().channel_count() > 1) {
		node& successor = output().connected_node();
		if(typeid(successor) != typeid(multiplex_node))
			throw invalid_node_graph("processing_node output with >1 channel must be connected to multiplex_node");
	}
}


void processing_node::handler_setup_() {
	Expects(handler_ != nullptr);
	handler_->handler_setup();
	// TODO verify completion of setup by handler
}


void processing_node::handler_pre_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	handler_->handler_pre_process(job);
}


void processing_node::handler_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	handler_->handler_process(job);
}


processing_node_job processing_node::begin_job_() {
	return processing_node_job(*this);
}


void processing_node::finish_job_(processing_node_job&) {
	///////
}


std::size_t processing_node::output_channel_count_() const noexcept {
	return output_channels_.size();
}


timed_frame_array_view processing_node::output_begin_read_(time_unit duration, std::ptrdiff_t channel_index) {
	return output_channels_.at(channel_index)->begin_read(duration);
}


void processing_node::output_end_read_(time_unit duration, std::ptrdiff_t channel_index) {
	return output_channels_.at(channel_index)->end_read(duration);
}


processing_node::processing_node(graph& gr, bool with_output) : node(gr) {
	if(with_output) add_output_<processing_node_output>();
}


processing_node::~processing_node() { }


void processing_node::set_handler(processing_node_handler& handler) {
	handler_ = &handler;
}


processing_node_input& processing_node::add_input() {
	return add_input_<processing_node_input>();
}


bool processing_node::has_output() const {
	return (output().size() == 1);
}


std::size_t processing_node::output_channel_count() const noexcept {
	if(has_output()) return output().channel_count();
	else return 0;
}	



///////////////


processing_node_job::processing_node_job(processing_node& nd) :
	time_(nd.current_time()),
	inputs_(nd.inputs().size()),
	output_channels_(nd.output_channel_count()) { }


processing_node_job::~processing_node_job() {
	cancel_inputs();
	cancel_outputs();
}


bool processing_node_job::open_input(processing_node_input& in) {
	std::ptrdiff_t index = in.index();
	timed_frame_array_view vw = in.begin_read_frame();
	if(vw.is_null()) return false;
	inputs_.at(index) = input_view_handle(&in, vw);
	return true;
}


void processing_node_job::commit_input(processing_node_input& in) {
	std::ptrdiff_t index = in.index();
}


void processing_node_job::cancel_inputs() {
	
}


bool processing_node_job::open_output(processing_node_output_channel& chan) {
	std::ptrdiff_t index = out.index();
	
}


void processing_node_job::commit_output(processing_node_output_channel& chan) {
	std::ptrdiff_t index = out.index();
	
}


void processing_node_job::cancel_outputs() {
	
}


bool processing_node_job::has_input_view(std::ptrdiff_t index) const noexcept {
	return ! is_null(inputs_.at(index));
}


const timed_frame_array_view& processing_node_job::input_view(std::ptrdiff_t index) {
	Expects(has_input_view(index));
	return inputs_.at(index).second;
}


bool processing_node_job::has_output_view(std::ptrdiff_t index) const noexcept {
	return ! is_null(outputs_.at(index));
}


const frame_view& processing_node_job::output_view(std::ptrdiff_t index) {
	Expects(has_output_view(index));
	return output_channels_.at(index).second;	
}

}}
