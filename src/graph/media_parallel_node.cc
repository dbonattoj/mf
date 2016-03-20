#include "media_parallel_node.h"
#include <functional>
#include <stdexcept>

namespace mf {

void media_parallel_node::thread_main_() {
	while(! reached_end_) {
		time_++;
			
		for(media_node_input_base* input : inputs_) {
			assert(! input->reached_end()); 
			input->begin_read(time_);
		}
		for(media_node_output_base* output : outputs_)
			output->begin_write();
	
		this->process_();
		if(this->process_reached_end_()) reached_end_ = true;
		
		for(media_node_input_base* input : inputs_) {
			input->end_read(time_);
			if(input->reached_end()) reached_end_ = true;
		}
		for(media_node_output_base* output : outputs_)
			output->end_write(reached_end_);
	}
}


media_parallel_node::media_parallel_node(time_unit prefetch) :
	media_node(prefetch)
{
	if(prefetch < 1) throw std::invalid_argument("prefetch must be at least 1");
}


media_parallel_node::~media_parallel_node() {
	
}


void media_parallel_node::pull(time_unit target_time) {
	// launch the thread if not yet done
	if(! thread_.joinable())
		thread_ = std::thread(std::bind(&media_parallel_node::thread_main_, this));
}


}