#include "media_node_base.h"

#include <algorithm>
#include "media_node_input.h"
#include "media_node_output.h"

namespace mf {

void media_node_base::register_input_(media_node_input_base& input) {
	inputs_.push_back(&input);
}


void media_node_base::register_output_(media_node_output_base& output) {
	outputs_.push_back(&output);
}


void media_node_base::propagate_offset_(time_unit new_offset) {
	if(new_offset <= offset_) return;
	
	offset_ = new_offset;
	
	for(media_node_input_base* input : inputs_) {
		media_node_base& connected_node = input->connected_output().node();
		time_unit off = offset_ + connected_node.prefetch_duration_ + input->future_window_duration();
		connected_node.propagate_offset_(off);
	}		
}


void media_node_base::propagate_output_buffer_durations_() {
	for(media_node_input_base* input : inputs_) {
		auto& output = input->connected_output();
		media_node_base& connected_node = output.node();
		time_unit dur = 1 + input->past_window_duration() + (connected_node.offset_ - offset_);
		output.define_required_buffer_duration(dur);
		connected_node.propagate_output_buffer_durations_();
	}
}


void media_node_base::propagate_setup_() {
	// do nothing when did_setup_ is already set:
	// during recursive propagation it may be called multiple times on same node
	if(did_setup_) return;
	
	// first set up preceding nodes
	for(media_node_input_base* input : inputs_) {
		media_node_base& connected_node = input->connected_output().node();
		connected_node.propagate_setup_();
	}
	
	// set up this node in concrete subclass
	this->setup_();
	
	// set up outputs
	// their frame shape are now defined, and required durations were defined before
	// (in propagate_output_buffer_durations_())
	for(media_node_output_base* output : outputs_) {
		if(! output->frame_shape_is_defined())
			throw std::logic_error("concrete subclass did not define output frame shapes");
		output->setup();
	}
	
	did_setup_ = true;
}


void media_node_base::propagate_stop_() {
	this->stop_();
	for(media_node_input_base* input : inputs_) {
		media_node_base& connected_node = input->connected_output().node();
		connected_node.propagate_stop_();
	}
}


void media_node_base::print(std::ostream& str) const {
	str << "node ";
	if(! name.empty()) str << '"' << name << '"';

	str << "[offset=" << offset_ << ", prefetch=" << prefetch_duration_ << ", setup=" << did_setup_ << "]" << std::endl;
	
	for(std::ptrdiff_t i = 0; i < inputs_.size(); ++i) {
		const media_node_input_base& input = *inputs_[i];
		str << "   input " << i
			<< ": [-" << input.past_window_duration() << ", +" << input.future_window_duration() << "]" << std::endl;
	}

	for(std::ptrdiff_t i = 0; i < outputs_.size(); ++i) {
		const media_node_output_base& output = *outputs_[i];
		str << "   output " << i
			<< ": required_duration=" << output.required_buffer_duration() << std::endl;
	}
}


}