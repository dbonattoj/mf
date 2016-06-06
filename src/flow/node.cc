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
#include "node_job.h"
#include <limits>

namespace mf { namespace flow {

time_unit node::minimal_offset_to(const node& target_node) const {
	time_unit minimum = std::numeric_limits<time_unit>::max();
	if(&target_node == this) return 0;
	for(auto&& out : outputs_) {
		const node_input& in = out->connected_input();
		if(in.this_node().precedes(target_node)) {
			time_unit off = in.this_node().minimal_offset_to(target_node) - in.past_window_duration();
			minimum = std::min(minimum, off);
		}
	}
	if(minimum != std::numeric_limits<time_unit>::max()) return minimum;
	else throw std::invalid_argument("tried to get minimal offset to preceding node");
}


time_unit node::maximal_offset_to(const node& target_node) const {
	time_unit maximum = std::numeric_limits<time_unit>::min();
	if(&target_node == this) return 0;
	for(auto&& out : outputs_) {
		const node_input& in = out->connected_input();
		if(in.this_node().precedes(target_node)) {
			time_unit off = in.this_node().maximal_offset_to(target_node) + in.future_window_duration() + prefetch_duration();
			maximum = std::max(maximum, off);
		}
	}
	if(maximum != std::numeric_limits<time_unit>::min()) return maximum;
	else throw std::invalid_argument("tried to get maximal offset to preceding node");
}


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



const node& node::first_successor_() const {
	return *this; // TODO
}



void node::deduce_stream_properties_() {
	MF_EXPECTS(! was_setup_);
	MF_EXPECTS(! is_source());
			
	seekable_ = true;
	stream_duration_ = std::numeric_limits<time_unit>::max();
	
	for(auto&& in : inputs()) {
		node& connected_node = in->connected_node();
		
		time_unit input_node_stream_duration = connected_node.stream_duration_;
		bool input_node_seekable = connected_node.seekable_;
		
		stream_duration_ = std::min(stream_duration_, input_node_stream_duration);
		seekable_ = seekable_ && input_node_seekable;
	}
	
	MF_ENSURES(!(seekable_ && (stream_duration_ == -1)));
}


void node::propagate_setup_() {	
	//MF_EXPECTS(min_offset_ != -1 && max_offset_ != -1); // ...were defined in prior setup phase
	
	// do nothing when was_setup_ is already set:
	// during recursive propagation it may be called multiple times on same node
	if(was_setup_) return;
	
	// first set up preceding nodes
	for(auto&& in : inputs()) {
		node& connected_node = in->connected_output().this_output().this_node();
		connected_node.propagate_setup_();
	}
	
	// define stream duration and seekable, based on connected input nodes
	if(! is_source()) deduce_stream_properties_();
	
	// set up this node in concrete subclass
	this->internal_setup();
	
	// set up outputs
	// their frame lengths are now defined
	for(auto&& out : outputs()) out->setup();
	
	was_setup_ = true;
}



void node::define_source_stream_properties(bool seekable, time_unit stream_duration) {
	MF_EXPECTS(! was_setup_);
	MF_EXPECTS(is_source());
	
	if(seekable && stream_duration == -1)
		throw std::invalid_argument("seekable node must have defined stream duration");

	seekable_ = seekable;
	stream_duration_ = stream_duration;
	end_time_ = stream_duration; // TODO combine end_time / stream_duration
}


void node::set_prefetch_duration(time_unit prefetch) {
	MF_EXPECTS(! was_setup_);
	MF_EXPECTS(prefetch >= 0);
	
	prefetch_duration_ = prefetch;
}


void node::setup_sink() {
	MF_EXPECTS(! was_setup_);
	MF_EXPECTS(is_sink());
	
	propagate_setup_();

	MF_ENSURES(was_setup_);
}


void node::propagate_inactive() {
	if(activation_ == inactive) return;
	if(std::none_of(outputs_.begin(), outputs_.end(), [](auto&& out) { return out->is_active(); })) {
		activation_ = inactive;
		for(auto&& in : inputs_) in->connected_node().propagate_inactive();
	}
}


void node::propagate_reactivating() {
	if(activation_ != inactive) return;
	if(std::any_of(outputs_.begin(), outputs_.end(), [](auto&& out) { return out->is_active(); })) {
		activation_ = reactivating;
		for(auto&& in : inputs_) in->connected_node().propagate_reactivating();
	}
}


void node::set_active() {
	activation_ = active;
}


bool node::is_bounded() const {
	if(stream_duration_ != -1 || is_source()) return true;
	else return std::any_of(inputs_.cbegin(), inputs_.cend(), [](auto&& in) {
		return (in->is_activated() && in->connected_node().is_bounded());
	});
}


node_job node::make_job() {
	return node_job(*this);
}


bool node::reached_end() const noexcept {
	return (end_time_ != -1) && (current_time_ >= end_time_ - 1);
}


/////


node_output::node_output(node& nd, std::ptrdiff_t index, const frame_format& format) :
	node_(nd), index_(index),
	format_(format) { }
	

void node_output::input_has_connected(node_input& input) {
	connected_input_ = &input;
}


bool node_output::is_active() const {
	if(connected_input_->is_activated() == false) return false;
	else return (connected_input_->this_node().activation() == node::active);
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


void node_input::pull(time_unit t) {
	MF_EXPECTS(is_connected());
	
	pull_time_ = t;

	time_unit start_time = std::max(time_unit(0), t - past_window_);
	time_unit end_time = t + future_window_ + 1;
	
	bool reactivate = (this_node().activation() == node::active) && (connected_node().activation() == node::reactivating);
	
	connected_output_->pull(time_span(start_time, end_time), reactivate);
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
	if(activated_ != activated) activated_ = activated;
	
	if(activated) connected_node().propagate_reactivating();
	else connected_node().propagate_inactive();
}

}}

