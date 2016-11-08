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
	this->setup();
	
	deduce_propagated_parameters_();
	deduce_sent_parameters_relay_();
	
	stage_ = stage::was_setup;
}


void node::setup_sink() {
	Assert(stage_ == stage::construction);
	Assert(is_sink());
	
	propagate_pre_setup_();
	propagate_setup_();

	Assert(stage_ == stage::was_setup);
}


void node::set_current_time_(time_unit t) {
	time_unit old_t = current_time_;
	current_time_ = t;
}


bool node::add_propagated_parameter_if_needed(parameter_id id) {	
	bool needed = false;
	for(auto&& out : outputs_) {
		bool needed_by_output = out->add_propagated_parameter_if_needed(id);
		if(needed_by_output) needed = true;
	}
	if(has_input_parameter(id)) needed = true;
	return needed;
}



bool node::add_relayed_parameter_if_needed(parameter_id id, const sent_parameter_relay_type& preceding_relay) {
	const sent_parameter_relay_type& default_relay = preceding_relay.handler(id);
	relayed_parameter_handler_type relay = this->custom_sent_parameter_relay_(id, default_relay);

	bool needed = false;
	for(auto&& out : outputs_) {
		relay = this->custom_sent_parameter_relay_(id, relay);		
		bool needed_by_output = out->add_relayed_parameter_if_needed(id, relay);
		if(needed_by_output) needed = true;
	}
	if(has_sent_parameter(id)) {
		needed = true;
		send_parameter_relays_.emplace(id, relay);
	}
	return needed;
}


node::sent_parameter_relay_type node::custom_sent_parameter_relay_
(parameter_id, const sent_parameter_relay_type& default_relay) {
	return default_relay;
}


sent_parameter_relay_type node::sent_parameter_relay(parameter_id id) const {
	auto it = sent_parameter_relays_.find(id);
	if(it != sent_parameter_relays_.end()) return it->second;
	else throw std::logic_error("relay for the sent parameter was not installed");
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



}}

