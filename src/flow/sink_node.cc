#include "sink_node.h"

namespace mf { namespace flow {
	
void sink_node::setup_graph() {
	setup_sink();
}


void sink_node::internal_setup() {
	if(outputs().size() != 0) throw invalid_flow_graph("sink_node cannot have outputs");
	this->setup();
}


void sink_node::launch() { }

void sink_node::stop() { }


void sink_node::pull(time_unit t) {
	// fail if reading past current_time, but end already reached
	if(t > current_time() && reached_end_) throw std::invalid_argument("cannot pull frame beyond end");
	else if(is_seekable()) reached_end_ = false;
	else throw std::invalid_argument("cannot pull frame before current time on non-seekable sink node");
	
	if(stream_duration_is_defined()) MF_ASSERT(t < stream_duration());
		
	// set pull = current time as requested
	set_pull_time(t);
	set_current_time(t);
	
	node_job job = make_job();
	job.define_time(t);
		
	// preprocess, allow concrete subclass to activate/desactivate inputs
	this->pre_process(t);
	// TODO job-like object passed in pre_process for input activation

	// pull & begin reading from activated inputs
	bool stopped = false;
	for(node_input& in : inputs()) {
		MF_ASSERT_MSG(! in.reached_end(t), "input of sink node must not already be at end");

		if(in.is_activated()) {
			in.pull(t);
			timed_frames_view in_view = in.begin_read_frame(t);
			if(in_view.is_null()) { stopped = true; break; }
			
			job.push_input(in, in_view);
		}
	}
	
	if(stopped) {
		// if stopped, close already-openned inputs and quit
		cancel_job(job);
		return;
	}

	// process frame in concrete subclass
	this->process(job);
	
	// end reading from inputs
	while(node_input* in = job.pop_input()) {
		in->end_read_frame(t);
		if(in->reached_end(t)) reached_end_ = true;
	}

	// if no activated inputs: stream duration determines end
	if(stream_duration_is_defined())
		if(t == stream_duration() - 1) reached_end_ = true;
}

// TODO (all nodes): handle cross over end when ended input desactivated

	
void sink_node::pull_next_frame() {
	pull(current_time() + 1);
}


void sink_node::seek(time_unit t) {
	pull(t);
}


}}
