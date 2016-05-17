#include "node.h"
#include "node_job.h"

namespace mf { namespace flow {

void node::propagate_offset_(time_unit new_offset) {
	MF_EXPECTS(! was_setup_);
	
	if(new_offset <= offset_) return;
	
	offset_ = new_offset;
	
	for(auto&& in : inputs()) {
		node& connected_node = in->connected_node();
		time_unit off = offset_ + connected_node.prefetch_duration_ + in->future_window_duration();
		connected_node.propagate_offset_(off);
	}		
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
	MF_EXPECTS(offset_ != -1); // ...was defined in prior setup phase
	
	// do nothing when was_setup_ is already set:
	// during recursive propagation it may be called multiple times on same node
	if(was_setup_) return;
	
	// first set up preceding nodes
	for(auto&& in : inputs()) {
		node& connected_node = in->connected_output().this_node();
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
	
	propagate_offset_(0);
	propagate_setup_();

	MF_ENSURES(was_setup_);
}


bool node::is_bounded() const {
	if(stream_duration_ != -1 || is_source()) return true;
	else return std::any_of(inputs_.cbegin(), inputs_.cend(), [](auto&& in) {
		return (in->is_activated() && in->connected_node().is_bounded());
	});
}


void node::update_activation() {
	// activation of succeeding nodes (down to sink) must already have been defined

	// this node is active if any of its outputs are active
	// an output is active, if its connected input is activated AND the connected node is active
	
	bool now_active = std::any_of(
		outputs_.cbegin(), outputs_.cend(),
		[](auto&& out) { return out->is_active(); }
	);

	if(now_active != active_) {
		active_ = now_active;

		// now set activation of preceeding nodes
		for(auto&& in : inputs()) in->connected_node().update_activation();
	}
}


node_job node::make_job() {
	return node_job(*this);
}


void node::cancel_job(node_job& job) {
	while(node_input* in = job.pop_input()) in->cancel_read_frame();
	while(node_output* out = job.pop_output()) out->cancel_write_frame();
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


void node_output::propagate_activation(bool active) {
	active_ = active;
	this_node().update_activation();
}


/////


node_input::node_input(node& nd, std::ptrdiff_t index, time_unit past_window, time_unit future_window) :
	node_(nd), index_(index),
	past_window_(past_window),
	future_window_(future_window) { }


void node_input::connect(node_output& output) {
	connected_output_ = &output;
	connected_output_->input_has_connected(*this);
}


void node_input::pull(time_unit t) {
	MF_EXPECTS(is_connected());

	time_unit start_time = std::max(time_unit(0), t - past_window_);
	time_unit end_time = t + future_window_ + 1;
		
	connected_output().pull(time_span(start_time, end_time));
}


timed_frame_array_view node_input::begin_read_frame(time_unit t) {
	time_unit duration = std::min(t, past_window_) + 1 + future_window_;
	timed_frame_array_view view = connected_output().begin_read(duration);
	return view;
}


void node_input::end_read_frame(time_unit t) {
	time_unit duration = (t < past_window_) ? 0 : 1;
	connected_output().end_read(duration);
}


void node_input::cancel_read_frame() {
	connected_output().end_read(0);
}


void node_input::set_activated(bool activated) {
	if(activated_ != activated) {
		activated_ = activated;
		
		bool output_active = activated && this_node().is_active();
		connected_output().propagate_activation(output_active);
	}
}

}}

