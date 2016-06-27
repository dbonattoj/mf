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
#include <unistd.h>

namespace mf { namespace flow {
	
void sink_node::setup_graph() {
	setup_sink();
}


void sink_node::setup() {
	if(outputs().size() != 0) throw invalid_flow_graph("sink_node cannot have outputs");
	setup_filter_();
}


void sink_node::pull(time_unit t) {
	//usleep(1000);
	//MF_DEBUG(t);
	
	// fail if reading past current_time, but end already reached
	if(t > current_time() && reached_end())
		throw std::invalid_argument("cannot pull frame beyond end");
	if(stream_properties().policy() != node_stream_properties::seekable && t <= current_time())
		throw std::invalid_argument("cannot pull frame before current time on non-seekable sink");

	//if(stream_duration_is_defined()) MF_ASSERT(t < stream_duration());
	
	filter_node_job job = make_job_(t);
		
	// set pull = current time as requested
	set_current_time_(t);
			
	// preprocess, allow concrete subclass to activate/desactivate inputs
	pre_process_filter_(job);

	// pull & begin reading from activated inputs
	bool stopped = false;
	for(auto&& in : inputs()) if(in->is_activated()) {
		time_unit res = in->pull(t);
		if(res == pull_result::stopped) {
			stopped = true;
			return;
		} else if(res == pull_result::temporary_failure) {
			MF_DEBUG("sink received temp failure");
			return;
		}
	}
	
	if(stopped) {
		return;
	}
	
	for(auto&& in : inputs()) if(in->is_activated()) {		
		bool cont = job.push_input(*in);
		if(! cont) {
			job.close_all_inputs();
			stopped = true;
			break;
		}
	}

	// process frame in concrete subclass
	process_filter_(job);
	

	while(job.has_inputs()) {
		const node_input& in = job.pop_input();
		if(t == in.end_time() - 1) mark_end_();
	}
	// if no activated inputs: stream duration determines end
	if(stream_properties().duration_is_defined())
		if(t == stream_properties().duration() - 1) mark_end_();
}

// TODO (all nodes): handle cross over end when ended input desactivated

	
bool sink_node::process_next_frame() {
	pull(current_time() + 1);
	return true;
}

void sink_node::seek(time_unit t) {
	MF_EXPECTS(stream_properties().policy() == node_stream_properties::seekable);
	if(t < 0 || t >= stream_properties().duration()) throw std::invalid_argument("seek target time out of bounds");
	set_current_time_(t - 1);
}


}}
