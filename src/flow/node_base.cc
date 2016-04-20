#include "node_base.h"

namespace mf { namespace flow {

void node_base::propagate_offset_(time_unit new_offset) {
	MF_EXPECTS(! was_setup_);
	
	if(new_offset <= offset_) return;
	
	offset_ = new_offset;
	
	for(input_base& in : inputs()) {
		node_base& connected_node = in.connected_node();
		time_unit off = offset_ + connected_node.prefetch_duration_ + in.future_window_duration();
		connected_node.propagate_offset_(off);
	}		
}


void node_base::propagate_output_buffer_durations_() {
	MF_EXPECTS(! was_setup_);
	for(input_base& in : inputs()) {
		output_base& out = in.connected_output();
		node_base& connected_node = out.node();
		time_unit dur = 1 + in.past_window_duration() + (connected_node.offset_ - offset_);
		out.define_required_buffer_duration(dur);
		connected_node.propagate_output_buffer_durations_();
	}
}


void node_base::deduce_stream_properties_() {
	MF_EXPECTS(! was_setup_);
	MF_EXPECTS(! is_source());
			
	seekable_ = true;
	stream_duration_ = std::numeric_limits<time_unit>::max();
	
	for(input_base& in : inputs()) {
		node_base& connected_node = in.connected_node();
		
		time_unit input_node_stream_duration = connected_node.stream_duration_;
		bool input_node_seekable = connected_node.seekable_;
		
		stream_duration_ = std::min(stream_duration_, input_node_stream_duration);
		seekable_ = seekable_ && input_node_seekable;
	}
	
	MF_ENSURES(!(seekable_ && (stream_duration_ == -1)));
}


void node_base::propagate_setup_() {	
	MF_EXPECTS(offset_ != -1); // ...was defined in prior setup phase
	
	// do nothing when was_setup_ is already set:
	// during recursive propagation it may be called multiple times on same node
	if(was_setup_) return;
	
	// first set up preceding nodes
	for(input_base& in : inputs()) {
		node_base& connected_node = in.connected_output().node();
		connected_node.propagate_setup_();
	}
	
	// define stream duration and seekable, based on connected input nodes
	if(! is_source()) deduce_stream_properties_();
	
	// set up this node in concrete subclass
	this->setup();
	
	// set up outputs
	// their frame shape are now defined
	// required durations were defined in propagate_output_buffer_durations_()
	// stream duration from node, was defined in propagate_stream_durations_()
	for(output_base& out : outputs()) out.setup();
	
	was_setup_ = true;
}


void node_base::propagate_activation_() {
	// activation of succeeding nodes (down to sink) must already have been defined

	// this node is active if any of its outputs are active
	// an output is active, if its connected input is activated AND the connected node is active
	
	bool now_active = std::any_of(
		outputs_.cbegin(), outputs_.cend(),
		[](output_base& out) { return out.is_active(); }
	);

	if(now_active != active_) {
		active_ = now_active;

		// now set activation of preceeding nodes
		for(input_base& in : inputs()) {
			node_base& connected_node = in.connected_output().node();
			connected_node.propagate_activation_();
		}
	}	
}


void node_base::define_source_stream_properties(bool seekable, time_unit stream_duration) {
	MF_EXPECTS(! was_setup_);
	MF_EXPECTS(is_source());
	
	if(seekable && stream_duration == -1)
		throw std::invalid_argument("seekable node must have stream duration");

	seekable_ = seekable;
	stream_duration_ = stream_duration;
}


void node_base::set_prefetch_duration(time_unit prefetch) {
	MF_EXPECTS(! was_setup_);
	MF_EXPECTS(prefetch >= 0);
	
	prefetch_duration_ = prefetch;
}


void node_base::setup_sink() {
	MF_EXPECTS(! was_setup_);
	MF_EXPECTS(is_sink());
	
	propagate_offset_(0);
	propagate_output_buffer_durations_();
	propagate_setup_();

	MF_ENSURES(was_setup_);
}


bool node_base::is_bounded() const {
	if(stream_duration_ != -1 || is_source()) return true;
	else return std::any_of(inputs_.cbegin(), inputs_.cend(), [](input_base& in) {
		return (in.is_activated() && in.connected_node().is_bounded());
	});
}


bool node_base::output_base::is_active() const noexcept {
	return input_activated_ && node_.is_active();
}


void node_base::input_base::set_activated(bool act) {
	if(activated_ != act) {
		activated_ = act;
		connected_output().propagate_activation(act);
	}
}


void node_base::output_base::propagate_activation(bool input_activated) {
	input_activated_ = input_activated;
	node_.propagate_activation_();
}


node_base::output_base::output_base(node_base& nd) : node_(nd) {
	nd.outputs_.emplace_back(*this);
}


node_base::input_base::input_base(node_base& nd, time_unit past_window, time_unit future_window) :
node_(nd), past_window_(past_window), future_window_(future_window) {
	nd.inputs_.emplace_back(*this);
}


}}

