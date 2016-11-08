#include "parameter_node.h"
#include <functional>

namespace mf { namespace flow {
	
parameter_node_output::parameter_node_output(node& nd) :
	node_output(nd),
	pulled_frame_(make_ndsize(1), nd.frame_format_) { }


const node_frame_format& parameter_node_output::frame_format() const {
	return this_node().frame_format_;
}


void parameter_node_output::pre_pull(const time_span& span) {
	Assert(span.duration() == 1);
}


node::pull_result parameter_node_output::pull(time_span& span) {
	Assert(span.duration() == 1);
	std::lock_guard<std::mutex> lock(this_node().parameters_mutex_);
	node_frame_view vw(pulled_frame_[0]);
	vw.satellite().propagated_parameters() = this_node().parameter_valuation_;
}


node_frame_window_view parameter_node_output::begin_read(time_unit duration) {
	return pulled_frame_[0];
}


void parameter_node_output::end_read(time_unit duration) {
	
}






///////////////


parameter_node::parameter_node(node_graph& gr) :
	base(gr), frame_format_() { }


node_parameter& parameter_node::add_parameter(parameter_id id, const node_parameter_value& initial_value) {
	parameters_.emplace_back(id, initial_value);
	parameter_valuation_.set(id, initial_value);
	return parameters_.back();
}


bool parameter_node::has_parameter(parameter_id id) const {
	return std::any_of(parameters_.cbegin(), parameters_.cend(), [id](const node_parameter& param) {
		return (param.id() == id);
	});
}


time_unit parameter_node::minimal_offset_to(const node&) const {
	throw not_implemented;


time_unit parameter_node::maximal_offset_to(const node&) const {
	throw not_implemented;
}
	

parameter_node_output& parameter_node::add_output() {
	parameter_node_output& out = add_output_(*this);
	out.set_name("out");
	return out;
}
	

void parameter_node::setup() {
	pulled_valuations_.reset(new timed_ring(frame_format_, 10));
	
	for(const node_parameter& param : parameters_) {
		add_propagated_parameter_if_needed(param.id());
		
		sent_parameter_relay_type relay = std::bind(parameter_node::update_parameter, this);
		add_relayed_parameter_if_needed(param.id(), relay);
	}
}


void parameter_node::update_parameter(parameter_id id, const node_parameter_value& val) {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	parameter_valuation_.set(id, val);
}


void parameter_node::update_parameter(parameter_id id, node_parameter_value&& val) {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	parameter_valuation_.set(id, std::move(val));
}


void parameter_node::update_parameters(const node_parameter_valuation& val) {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	parameter_valuation_.set_all(val);
}


void parameter_node::update_parameters(node_parameter_valuation&& val) {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	parameter_valuation_.set_all(std::move(val));
}


node_parameter_valuation parameter_node::current_parameter_valuation() const {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	return parameter_valuation_;
}

}}
