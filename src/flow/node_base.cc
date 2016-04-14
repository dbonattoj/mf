#include "node_base.h"

#include <algorithm>
#include <limits>
#include "node_input.h"
#include "node_output.h"

namespace mf { namespace flow {
	

void node_base::define_source_stream_properties(bool seekable, time_unit stream_duration) {
	assert(is_source());
	
	if(was_setup_) throw std::logic_error("node was already set up");
	if(seekable && stream_duration == -1)
		throw std::invalid_argument("seekable node must have defined stream duration");

	seekable_ = seekable;
	stream_duration_ = stream_duration;
}


void node_base::register_input_(node_input_base& input) {
	inputs_.push_back(&input);
}


void node_base::register_output_(node_output_base& output) {
	outputs_.push_back(&output);
}


void node_base::propagate_offset_(time_unit new_offset) {
	if(new_offset <= offset_) return;
	
	offset_ = new_offset;
	
	for(node_input_base* input : inputs_) {
		node_base& connected_node = input->connected_output().node();
		time_unit off = offset_ + connected_node.prefetch_duration_ + input->future_window_duration();
		connected_node.propagate_offset_(off);
	}		
}


void node_base::propagate_output_buffer_durations_() {
	for(node_input_base* input : inputs_) {
		auto& output = input->connected_output();
		node_base& connected_node = output.node();
		time_unit dur = 1 + input->past_window_duration() + (connected_node.offset_ - offset_);
		output.define_required_buffer_duration(dur);
		connected_node.propagate_output_buffer_durations_();
	}
}


void node_base::deduce_stream_properties_() {
	assert(! is_source());
		
	seekable_ = true;
	stream_duration_ = std::numeric_limits<time_unit>::max();
	
	for(node_input_base* input : inputs_) {
		auto& output = input->connected_output();
		node_base& connected_node = output.node();
		
		time_unit input_node_stream_duration = connected_node.stream_duration_;
		bool input_node_seekable = connected_node.seekable_;
		
		stream_duration = std::min(stream_duration, input_node_stream_duration);
		seekable_ = seekable_ && input_node_seekable;
	}
	
	assert(!(seekable_ && (stream_duration_ == -1)));
}


void node_base::propagate_setup_() {
	assert(offset_ != -1); // ...was defined in prior setup phase
	
	// do nothing when did_setup_ is already set:
	// during recursive propagation it may be called multiple times on same node
	if(did_setup_) return;
	
	// first set up preceding nodes
	for(node_input_base* input : inputs_) {
		node_base& connected_node = input->connected_output().node();
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
	for(node_output_base* output : outputs_) {
		if(! output->frame_shape_is_defined())
			throw std::logic_error("concrete subclass did not define output frame shapes");

		assert(output->required_buffer_duration_is_defined()); // ...was defined in prior setup phase
					
		output->setup();
	}
	
	did_setup_ = true;
}


bool node_base::is_active() const {
	// node is always active if it has no outputs (i.e. sink node)
	if(is_sink()) return true;
	
	// otherwise, it is active if any output is active
	for(node_output_base* output : outputs_)
		if(output->is_active()) return true;
		
	return false;
}



#ifndef NDEBUG
void node_base::debug_print(std::ostream& str) const {
	str << "node ";
	if(! name.empty()) str << '"' << name << '"';

	str << "[offset=" << offset_ << ", prefetch=" << prefetch_duration_ << ", setup=" << did_setup_ << "]" << std::endl;
	
	for(std::ptrdiff_t i = 0; i < inputs_.size(); ++i) {
		const node_input_base& input = *inputs_[i];
		str << "   input " << i
			<< ": [-" << input.past_window_duration() << ", +" << input.future_window_duration() << "]" << std::endl;
	}

	for(std::ptrdiff_t i = 0; i < outputs_.size(); ++i) {
		const node_output_base& output = *outputs_[i];
		str << "   output " << i
			<< ": required_duration=" << output.required_buffer_duration() << std::endl;
	}
}
#endif

}}
