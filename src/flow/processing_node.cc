#include "processing_node.h"

namespace mf { namespace flow {


std::size_t processing_node_output::channels_count() const noexcept {
	return this_node().output_channels_count();
}


node::pull_result processing_node_output::pull(time_span& span, bool reconnect) {
	return this_node().output_pull_(span, reconnect);
}


timed_frame_array_view processing_node_output::begin_read(time_unit duration) {
	return this_node().output_begin_read_(duration);
}


void processing_node_output::end_read(time_unit duration) {
	return this_node().output_end_read_(duration);
}


///////////////


void processing_node::verify_connections_validity_() const {
	if(outputs().size() > 1) throw invalid_node_graph("processing_node must have at most 1 output");
	
	if(has_output() && output_channels_count() > 1) {
		node& successor = output().connected_node();
		if(typeid(successor) != typeid(multiplex_node))
			throw invalid_node_graph("processing_node output with >1 channel must be connected to multiplex_node");
	}
}


void processing_node::handler_setup_() {
	Expects(handler_ != nullptr);
	handler_->handler_setup();
	
	for(const output_channel_type& chan : output_channels_)
		if(! chan.format().is_defined()) throw invalid_node_graph("processing_node did not set output channel format");
}


void processing_node::handler_pre_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	handler_->handler_pre_process(job);
}


void processing_node::handler_process_(processing_node_job& job) {
	Expects(handler_ != nullptr);
	handler_->handler_process(job);
}


processing_node_job processing_node::begin_job_(time_unit t) {
	return processing_node_job(*this);
}


void processing_node::finish_job_(processing_node_job& job) {
	bool reached_end = false;
	
	if(stream_properties().duration_is_defined()
		&& current_time() == stream_properties().duration() - 1) reached_end = true;
	else if(job.end_was_marked()) reached_end = true;
	
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		if(job.has_input_view(i)) {
			const node_input& in = *inputs().at(i);
			if(current_time() == in.end_time() - 1) reached_end = true;
			job.end_input(i);
		}
	}
		
	if(reached_end) mark_end_();
}


std::size_t processing_node::output_channels_count_() const noexcept {
	return output_channels_.size();
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


processing_node_output_channel& processing_node::add_output_channel() {
	std::ptrdiff_t channel_index = output_channels_.size();
	output_channels_.emplace_back(*this, channel_index);
	return output_channels_.back();
}


bool processing_node::has_output() const {
	return (output().size() == 1);
}


std::size_t processing_node::output_channel_count() const noexcept {
	if(has_output()) return output().channel_count();
	else return 0;
}


processing_node_output_channel& processing_node::output_channel_at(std::ptrdiff_t index) {
	return output_channels_.at(index);
}


const processing_node_output_channel& processing_node::output_channel_at(std::ptrdiff_t index) const {
	return output_channels_.at(index);
}



///////////////


processing_node_job::processing_node_job(processing_node& nd) :
	node_(nd),
	inputs_(nd.inputs().size()) { }


processing_node_job::~processing_node_job() {
	Expects(output_view_.is_null(), "processing_node_job must be detached prior to destruction");
	cancel_inputs();
}


bool processing_node_job::begin_input(processing_node_input& in) {
	std::ptrdiff_t index = in.index();
	timed_frame_array_view vw = in.begin_read_frame();
	if(vw.is_null()) return false;
	inputs_.at(index) = input_view_handle(&in, vw);
	return true;
}


void processing_node_job::end_input(processing_node_input& in) {
	std::ptrdiff_t index = in.index();
	in.end_read_frame();	
	set_null_(inputs_.at(index));
}


void processing_node_job::cancel_inputs() {
	for(input_view_handle& in : inputs_) if(! is_null_(in)) {
		in.cancel_read_frame();	
		set_null_(in);
	}
}


void processing_node_job::attach_output_view(const frame_view& out_vw) {
	output_view_.reset(out_vw);
}


void processing_node_job::detach_output_view() {
	output_view_.reset();
}


bool processing_node_job::has_input_view(std::ptrdiff_t index) const noexcept {
	return ! is_null(inputs_.at(index));
}


const timed_frame_array_view& processing_node_job::input_view(std::ptrdiff_t index) const {
	Expects(has_input_view(index));
	return inputs_.at(index).second;
}

const frame_view& processing_node_job::output_view() const {
	Expects(has_output_view());
	return output_view_;
}

}}
