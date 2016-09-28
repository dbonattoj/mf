/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "node.h"
#include "node_output.h"
#include "node_input.h"
#include "node_graph.h"
#include <limits>
#include <algorithm>
#include <set>

namespace mf { namespace flow {

node::~node() { }

bool node::precedes(const node& nd) const {
	if(&nd == this) return true;
	for(auto&& out : outputs_)
		if(out->connected_node().precedes(nd)) return true;
	return false;
}



bool node::precedes_strict(const node& nd) const {
	if(&nd == this) return false;
	for(auto&& out : outputs_)
		if(out->connected_node().precedes(nd)) return true;
	return false;
}


const node& node::first_successor() const {	
	Assert(outputs_.size() > 0);
	
	if(outputs_.size() == 1) return outputs_.front()->connected_node();
	
	using nodes_vector_type = std::set<const node*>;
	
	// collect_all_successors(nd, vec): adds ptrs to all successor nodes of `nd` (and also `nd` itself) into `vec`
	std::function<void(const node&, nodes_vector_type&)> collect_all_successors;
	collect_all_successors = [&](const node& nd, nodes_vector_type& vec) {
		vec.insert(&nd);
		for(auto&& out : nd.outputs_) {
			const node& connected_node = out->connected_node();
			collect_all_successors(connected_node, vec);
		}
	};

	// common_successors := successors of node connected to first output
	nodes_vector_type common_successors;
	collect_all_successors(outputs_.front()->connected_node(), common_successors);
	
	// for the other outputs...
	for(auto it = outputs_.cbegin() + 1; it < outputs_.cend(); ++it) {
		// out_successors := successors of node connected to output `it`
		const node& connected_node = (*it)->connected_node();
		nodes_vector_type out_successors;
		collect_all_successors(connected_node, out_successors);	
		
		// common_successors := intersection(common_successors, out_successors)
		nodes_vector_type old_common_successors = common_successors;
		common_successors.clear();
		std::set_intersection(
			old_common_successors.cbegin(), old_common_successors.cend(),
			out_successors.cbegin(), out_successors.cend(),
			std::inserter(common_successors, common_successors.begin())
		);
	}
	Assert(common_successors.size() > 0);
	// common_successors = nodes that are successors of every output
		
	// find node in `common_successors` that is not preceded by any other
	// (precedes_strict forms a partial order)
	auto it = std::find_if(
		common_successors.cbegin(), common_successors.cend(),
		[&common_successors](const node* a) { return std::none_of(
			common_successors.cbegin(), common_successors.cend(),
			[a](const node* b) { return b->precedes_strict(*a); }
		); }
	);
	Assert(it != common_successors.cend());

	return **it;
}


void node::deduce_stream_timing_() {
	Expects(stage_ == stage::was_pre_setup);
	Expects(! is_source());
	
	// assume inputs are either all realtime or all non-realtime
	bool realtime = input_at(0).connected_node().stream_timing().is_real_time();

	if(realtime) {
		stream_timing_.set_real_time(true);
	} else {
		bool has_duration = true;
		time_unit duration = std::numeric_limits<time_unit>::max();

		for(auto&& in : inputs()) {
			node& connected_node = in->connected_node();
			const node_stream_timing& connected_tm = connected_node.stream_timing();	
		
			if(connected_tm.has_duration()) duration = std::min(duration, connected_tm.duration());
			else has_duration = false;
		}
		
		stream_timing_.set_real_time(false);
		if(has_duration) stream_timing_.set_duration(duration);
		else stream_timing_.set_no_duration();
	}
}



void node::propagate_pre_setup_() {
	// do nothing if this node was already pre_setup
	if(stage_ == stage::was_pre_setup) return;
	
	// do nothing if any of its direct successors are not yet pre_setup	
	for(auto&& out : outputs()) {
		const node& direct_successor = out->connected_node();
		if(direct_successor.stage_ != stage::was_pre_setup) return;
	}

	// pre_setup this node
	Assert(stage_ == stage::construction);
	this->pre_setup();
	stage_ = stage::was_pre_setup;
	
	// recursively attempt to pre_setup direct predecessors
	for(auto&& in : inputs()) {
		node& direct_predecessor = in->connected_node();
		direct_predecessor.propagate_pre_setup_();
	}
}


void node::propagate_setup_() {	
	// do nothing if this node was already setup
	if(stage_ == stage::was_setup) return;
	
	// recursively attempt to setup its direct predecessors
	for(auto&& in : inputs()) {
		node& direct_predecessor = in->connected_node();
		direct_predecessor.propagate_setup_();
	}

	// setup this node
	Assert(stage_ == stage::was_pre_setup);
	if(! is_source()) deduce_stream_timing_();
	this->setup();
	
	deduce_propagated_parameters_();
	deduce_sent_parameters_relay_();
	
	stage_ = stage::was_setup;
}



void node::define_source_stream_timing(const node_stream_timing& tm) {
	Expects(stage_ == stage::construction);
	Expects(is_source());
	
	stream_timing_ = tm;
}


void node::setup_sink() {
	Expects(stage_ == stage::construction);
	Expects(is_sink());
	
	propagate_pre_setup_();
	propagate_setup_();

	Ensures(stage_ == stage::was_setup);
}


bool node::is_bounded() const {
	if(stream_timing_.has_duration() || is_source()) return true;
	else return std::any_of(inputs_.cbegin(), inputs_.cend(), [](auto&& in) {
		return (in->is_activated() && in->connected_node().is_bounded());
	});
}


void node::set_current_time_(time_unit t) noexcept {
	time_unit old_t = current_time_;
	current_time_ = t;
		
	if(t < old_t) reached_end_ = false;
}


void node::mark_end_() {
	reached_end_ = true;
}


time_unit node::end_time() const noexcept {
	if(stream_timing().has_duration()) return stream_timing().duration();
	else if(reached_end()) return current_time() + 1;
	else return -1;
}


void node::deduce_propagated_parameters_() {
	for(const node_parameter& param : parameters_)
		add_propagated_parameter_if_needed(param.id());
}


void node::deduce_sent_parameters_relay_() {
	node_parameter_relay null_relay;
	for(const node_parameter& param : parameters_)
		add_relayed_parameter_if_needed(param.id(), null_relay);
	Assert(null_relay.handlers_count() == 0);
}


bool node::add_propagated_parameter_if_needed(parameter_id id) {
	std::cout << "node(" << name_ << ")::add_propagated_parameter_if_needed(" << id << ")" << std::endl;
	
	bool needed = false;
	for(auto&& out : outputs_) {
		bool needed_by_output = out->add_propagated_parameter_if_needed(id);
		if(needed_by_output) needed = true;
	}
	if(has_input_parameter(id)) needed = true;
	return needed;
}



bool node::add_relayed_parameter_if_needed(parameter_id id, const node_parameter_relay& preceding_relay) {
	bool needed = false;
	for(auto&& out : outputs_) {
		bool needed_by_output = out->add_relayed_parameter_if_needed(id, sent_parameters_relay_);
		if(needed_by_output) needed = true;
	}
	if(has_sent_parameter(id)) needed = true;
	
	if(has_parameter(id)) {
		needed = true;
		sent_parameters_relay_.set_handler(
			id,
			[id, this](const node_parameter_value& val) { update_parameter_(id, val); }
		);
	
	} else if(needed) {
		sent_parameters_relay_.set_handler(
			id,
			[id, &preceding_relay](const node_parameter_value& val) { preceding_relay.send_parameter(id, val); }
		);
	}
	
	return needed;
}



node_parameter& node::add_parameter(parameter_id id, const node_parameter_value& initial_value) {
	parameters_.emplace_back(id, initial_value);
	parameter_valuation_.set(id, initial_value);
	return parameters_.back();
}


bool node::has_parameter(parameter_id id) const {
	return std::any_of(parameters_.cbegin(), parameters_.cend(), [id](const node_parameter& param) {
		return (param.id() == id);
	});
}


void node::add_input_parameter(parameter_id id) {
	input_parameters_.push_back(id);
}


bool node::has_input_parameter(parameter_id id) const {
	return std::find(input_parameters_.cbegin(), input_parameters_.cend(), id) != input_parameters_.cend();
}


void node::add_sent_parameter(parameter_id id) {
	sent_parameters_.push_back(id);
}


bool node::has_sent_parameter(parameter_id id) const {
	return std::find(sent_parameters_.cbegin(), sent_parameters_.cend(), id) != sent_parameters_.cend();
}


void node::update_parameter_(parameter_id id, const node_parameter_value& val) {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	parameter_valuation_.set(id, val);
}


void node::update_parameter_(parameter_id id, node_parameter_value&& val) {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	parameter_valuation_.set(id, std::move(val));
}


void node::update_parameters_(const node_parameter_valuation& val) {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	parameter_valuation_.set_all(val);
}


void node::update_parameters_(node_parameter_valuation&& val) {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	parameter_valuation_.set_all(std::move(val));
}


node_parameter_valuation node::current_parameter_valuation_() const {
	std::lock_guard<std::mutex> lock(parameters_mutex_);
	return parameter_valuation_;
}



}}

