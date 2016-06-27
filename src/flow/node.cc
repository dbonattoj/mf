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
#include <limits>
#include <algorithm>

namespace mf { namespace flow {


bool node::precedes(const node& nd) const {
	if(&nd == this) return true;
	for(auto&& out : outputs_) if(out->connected_input().this_node().precedes(nd)) return true;
	return false;
}



bool node::precedes_strict(const node& nd) const {
	if(&nd == this) return false;
	for(auto&& out : outputs_) if(out->connected_input().this_node().precedes(nd)) return true;
	return false;
}



const node& node::first_successor() const {
	if(outputs_.size() == 1) return outputs_.front()->connected_node();
	
	using nodes_vector_type = std::vector<const node*>;
	
	std::function<void(const node&, nodes_vector_type&)> collect_all_successors;
	collect_all_successors = [&](const node& nd, nodes_vector_type& vec) {
		for(auto&& out : nd.outputs_) {
			const node& connected_node = out->connected_node();
			vec.push_back(&connected_node);
			collect_all_successors(connected_node, vec);
		}
	};

	nodes_vector_type common_successors;
	collect_all_successors(outputs_.front()->connected_node(), common_successors);
	
	for(auto it = outputs_.cbegin() + 1; it != outputs_.cend(); ++it) {
		const node& connected_node = (*it)->connected_node();
		nodes_vector_type out_successors;
		collect_all_successors(connected_node, out_successors);
				
		nodes_vector_type old_common_successors = common_successors;
		common_successors.clear();
		std::set_intersection(
			old_common_successors.cbegin(), old_common_successors.cend(),
			out_successors.cbegin(), out_successors.cend(),
			std::back_inserter(common_successors)
		);
	}

	auto it = std::min_element(
		common_successors.cbegin(), common_successors.cend(),
		[](const node* a, const node* b) { return a->precedes(*b); }
	);
	
	
	return **it;
}


void node::deduce_stream_properties_() {
	MF_EXPECTS(stage_ == was_setup);
	MF_EXPECTS(! is_source());

	bool seekable = true;
	time_unit duration = std::numeric_limits<time_unit>::max();

	for(auto&& in : inputs()) {
		node& connected_node = in->connected_node();
		const node_stream_properties& connected_prop = connected_node.stream_properties();	
		bool connected_node_seekable = (connected_prop.policy() == node_stream_properties::seekable);
		
		duration = std::min(duration, connected_prop.duration());
		seekable = seekable && connected_node_seekable;
	}

	MF_ASSERT(!(seekable && (duration == -1)));
	
	auto policy = (seekable ? node_stream_properties::seekable : node_stream_properties::forward);
	stream_properties_ = node_stream_properties(policy, duration);
}


void node::propagate_pre_setup_() {
	if(stage_ == was_pre_setup) return;
		
	for(auto&& in : inputs()) {
		node& connected_node = in->connected_node();
		connected_node.propagate_pre_setup_();
	}
	
	this->pre_setup();
	
	stage_ = was_pre_setup;
}



void node::propagate_setup_() {		
	if(stage_ != was_setup) return;
	
	for(auto&& in : inputs()) {
		node& connected_node = in->connected_node();
		connected_node.propagate_setup_();
	}
	
	if(! is_source()) deduce_stream_properties_();

	this->setup();
	
	stage_ = was_pre_setup;
}



void node::define_source_stream_properties(const node_stream_properties& prop) {
	MF_EXPECTS(stage_ == construction);
	MF_EXPECTS(is_source());
	
	stream_properties_ = prop;
}


void node::setup_sink() {
	MF_EXPECTS(stage_ == construction);
	MF_EXPECTS(is_sink());
	
	propagate_pre_setup_();
	propagate_setup_();

	MF_EXPECTS(stage_ == was_setup);
}


void node::propagate_offline_state() {
	if(state_ == offline) return;
	if(std::none_of(outputs_.begin(), outputs_.end(), [](auto&& out) { return out->is_online(); })) {
		state_ = offline;
		for(auto&& in : inputs_) in->connected_node().propagate_offline_state();
	}
}


void node::propagate_reconnecting_state() {
	if(state_ != offline) return;
	if(std::any_of(outputs_.begin(), outputs_.end(), [](auto&& out) { return out->is_online(); })) {
		state_ = reconnecting;
		for(auto&& in : inputs_) in->connected_node().propagate_reconnecting_state();
	}
}


void node::set_online() {
	state_ = online;
}


bool node::is_bounded() const {
	if(stream_properties_.duration_is_defined() || is_source()) return true;
	else return std::any_of(inputs_.cbegin(), inputs_.cend(), [](auto&& in) {
		return (in->is_activated() && in->connected_node().is_bounded());
	});
}


/////


node_output::node_output(node& nd, std::ptrdiff_t index, const frame_format& format) :
	node_(nd), index_(index),
	format_(format) { }
	

void node_output::input_has_connected(node_input& input) {
	connected_input_ = &input;
}


bool node_output::is_online() const {
	if(connected_input_->is_activated() == false) return false;
	else return (connected_input_->this_node().state() == node::online);
}


node& node_output::connected_node() const noexcept {
	return connected_input_->this_node();
}


time_unit node_output::end_time() const noexcept {
	if(this_node().reached_end()) return this_node().current_time();
	else return -1;
}


/////


node_input::node_input(node& nd, std::ptrdiff_t index, time_unit past_window, time_unit future_window) :
	node_(nd), index_(index),
	past_window_(past_window),
	future_window_(future_window) { }


void node_input::connect(node_remote_output& output) {
	connected_output_ = &output;
	connected_output_->this_output().input_has_connected(*this);
}


node::pull_result node_input::pull(time_unit t) {
	MF_EXPECTS(is_connected());
	
	time_unit start_time = std::max(time_unit(0), t - past_window_);
	time_unit end_time = t + future_window_ + 1;
	
	bool reconnect = (this_node().state() == node::online) && (connected_node().state() == node::reconnecting);
	
	time_span span(start_time, end_time);
	return connected_output_->pull(span, reconnect);
}


timed_frame_array_view node_input::begin_read_frame(time_unit t) {
	time_unit duration = std::min(t, past_window_) + 1 + future_window_;
	timed_frame_array_view view = connected_output_->begin_read(duration);
	return view;
}


void node_input::end_read_frame(time_unit t) {
	time_unit duration = (t < past_window_) ? 0 : 1;
	connected_output_->end_read(duration);
}


void node_input::cancel_read_frame() {
	connected_output_->end_read(0);
}


void node_input::set_activated(bool activated) {
	if(activated_ != activated) {
		activated_ = activated;
		if(activated) connected_node().propagate_reconnecting_state();
		else connected_node().propagate_offline_state();
	}
}

}}

