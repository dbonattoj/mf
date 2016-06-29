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
	Expects(outputs_.size() > 0);
	
	if(outputs_.size() == 1) return outputs_.front()->connected_node();
	
	using nodes_vector_type = std::vector<const node*>;
	
	// collect_all_successors(nd, vec): adds ptrs to all successor nodes of `nd` into `vec`
	std::function<void(const node&, nodes_vector_type&)> collect_all_successors;
	collect_all_successors = [&](const node& nd, nodes_vector_type& vec) {
		for(auto&& out : nd.outputs_) {
			const node& connected_node = out->connected_node();
			vec.push_back(&connected_node);
			collect_all_successors(connected_node, vec);
		}
	};

	// common_successors := successors of node connected to first output
	nodes_vector_type common_successors;
	collect_all_successors(outputs_.front()->connected_node(), common_successors);
	
	// for the other outputs...
	for(auto it = outputs_.cbegin() + 1; it != outputs_.cend(); ++it) {
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
			std::back_inserter(common_successors)
		);
	}
	// common_successors = nodes that are successors of every output
	
	// find node in `common_successors` that is not preceded by any another
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


void node::deduce_stream_properties_() {
	Expects(stage_ == was_pre_setup);
	Expects(! is_source());

	bool seekable = true;
	time_unit duration = std::numeric_limits<time_unit>::max();

	for(auto&& in : inputs()) {
		node& connected_node = in->connected_node();
		const node_stream_properties& connected_prop = connected_node.stream_properties();	
		bool connected_node_seekable = (connected_prop.policy() == node_stream_properties::seekable);
		
		duration = std::min(duration, connected_prop.duration());
		seekable = seekable && connected_node_seekable;
	}

	Assert(!(seekable && (duration == -1)));
	
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
	if(stage_ == was_setup) return;
	
	for(auto&& in : inputs()) {
		node& connected_node = in->connected_node();
		connected_node.propagate_setup_();
	}
	
	if(! is_source()) deduce_stream_properties_();

	this->setup();
	
	stage_ = was_setup;
}



void node::define_source_stream_properties(const node_stream_properties& prop) {
	Expects(stage_ == construction);
	Expects(is_source());
	
	stream_properties_ = prop;
}


void node::setup_sink() {
	Expects(stage_ == construction);
	Expects(is_sink());
	
	propagate_pre_setup_();
	propagate_setup_();

	Ensures(stage_ == was_setup);
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


void node::set_current_time_(time_unit t) noexcept {
	time_unit old_t = current_time_;
	current_time_ = t;
		
	if(t < old_t) reached_end_ = false;
}


void node::mark_end_() {
	reached_end_ = true;
}


time_unit node::end_time() const noexcept {
	if(stream_properties().duration_is_defined()) return stream_properties().duration();
	else if(reached_end()) return current_time() + 1;
	else return -1;
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
	return this_node().end_time();
}


/////


node_input::node_input(node& nd, std::ptrdiff_t index, time_unit past_window, time_unit future_window) :
	node_(nd), index_(index),
	past_window_(past_window),
	future_window_(future_window) { }


void node_input::connect(node_remote_output& output) {
	Expects(! output.this_output().is_connected(), "cannot connect the input to an output that is already connected");
	
	connected_output_ = &output;
	connected_output_->this_output().input_has_connected(*this);
}


node::pull_result node_input::pull() {
	Expects(is_connected());
	
	time_unit t = this_node().current_time();
	time_unit start_time = std::max(time_unit(0), t - past_window_);
	time_unit end_time = t + future_window_ + 1;
	
	bool reconnect = (this_node().state() == node::online) && (connected_node().state() == node::reconnecting);
	// TODO check when connected_node().state() can change
	
	time_span span = time_span(start_time, end_time);
	node::pull_result result = connected_output_->pull(span, reconnect);
	Assert(span.start_time() == start_time);
	Assert(span.includes(t));
	
	pulled_span_ = span;
	return result;
}


timed_frame_array_view node_input::begin_read_frame() {
	Expects(pulled_span_.duration() > 0);

	time_unit t = this_node().current_time();
	time_unit duration = std::min(t, past_window_) + 1 + future_window_;
	if(duration > pulled_span_.duration()) duration = pulled_span_.duration();
	
	//duration = pulled_span_.duration();
	
	timed_frame_array_view view = connected_output_->begin_read(duration);
	Assert(! view.is_null());
	
	if(view.is_null()) return view;
	
	Assert(view.span().includes(t));
	Assert(view.start_time() == pulled_span_.start_time());
	Assert(view.duration() == duration);
	return view;
}


void node_input::end_read_frame() {
	time_unit duration = (this_node().current_time() < past_window_) ? 0 : 1;
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

