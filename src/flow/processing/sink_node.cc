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

#include "sink_node.h"
#include "../node_graph.h"

namespace mf { namespace flow {
	
sink_node::sink_node(node_graph& gr) : processing_node(gr, false) {
	set_name("sink");
}


thread_index sink_node::processing_thread_index() const {
	return graph().root_thread_index();
}


void sink_node::setup_graph() {
	setup_sink();
}


void sink_node::setup() {
	if(outputs().size() != 0) throw invalid_flow_graph("sink_node cannot have outputs");
	handler_setup_();
}


void sink_node::pull(time_unit t) {
	//usleep(1000);
	//MF_DEBUG(t);
	
	// fail if reading past current_time, but end already reached
	if(t > current_time() && reached_end())
		throw std::invalid_argument("cannot pull frame beyond end");

	//if(stream_duration_is_defined()) MF_ASSERT(t < stream_duration());
	
	set_current_time_(t);
	processing_node_job job = begin_job_();
		
	// set pull = current time as requested
			
	// preprocess, allow concrete subclass to activate/desactivate inputs
	handler_pre_process_(job);

	// pull & begin reading from activated inputs
	bool stopped = false;
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(! in.is_activated()) continue;

		time_unit res = in.pull();
		if(res == pull_result::stopped) {
			stopped = true;
			return;
		} else if(res == pull_result::transitory_failure) {
			MF_DEBUG("sink received temp failure");
			throw sequencing_error("stf");
			return;
		}
	}
	
	if(stopped) {
		return;
	}
	
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(! in.is_activated()) continue;

		bool cont = job.begin_input(in);
		if(! cont) {
			job.cancel_inputs();
			stopped = true;
			break;
		}
	}

	// process frame in concrete subclass
	handler_process_(job);
	
	finish_job_(job);
}


// TODO (all nodes): handle cross over end when ended input desactivated

	
bool sink_node::process_next_frame() {
	pull(current_time() + 1);
	return true;
}

void sink_node::seek(time_unit t) {
	if(t < 0 || t >= stream_timing().duration()) throw std::invalid_argument("seek target time out of bounds");
	set_current_time_(t - 1);
	
}


}}
