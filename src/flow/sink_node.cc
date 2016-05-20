#include "sink_node.h"
#include "node_job.h"

namespace mf { namespace flow {
	
void sink_node::setup_graph() {
	setup_sink();
}


void sink_node::internal_setup() {
	if(outputs().size() != 0) throw invalid_flow_graph("sink_node cannot have outputs");
	setup_filter();
}


void sink_node::launch() { }

void sink_node::stop() { }


void sink_node::pull(time_unit t) {
	// fail if reading past current_time, but end already reached
	if(t > current_time() && reached_end())
		throw std::invalid_argument("cannot pull frame beyond end");
	if(!is_seekable() && t <= current_time())
		throw std::invalid_argument("cannot pull frame before current time on non-seekable sink");

	if(stream_duration_is_defined()) MF_ASSERT(t < stream_duration());
		
	// set pull = current time as requested
	set_current_time(t);
	
	node_job job = make_job();
	job.define_time(t);
		
	// preprocess, allow concrete subclass to activate/desactivate inputs
	pre_process_filter(job);

	// pull & begin reading from activated inputs
	bool stopped = false;
	for(auto&& in : inputs()) {
		if(in->is_activated()) {
			in->pull(t);
			timed_frame_array_view in_view = in->begin_read_frame(t);
			if(in_view.is_null()) { stopped = true; break; }
			
			job.push_input(*in, in_view);
		}
	}
	
	if(stopped) {
		// if stopped, close already-openned inputs and quit
		cancel_job(job);
		return;
	}

	// process frame in concrete subclass
	process_filter(job);
	
	bool reached_end = false;
	// end reading from inputs
	while(node_input* in = job.pop_input()) {
		in->end_read_frame(t);
		if(t == in->end_time() - 1) reached_end = true;
	}

	// if no activated inputs: stream duration determines end
	if(stream_duration_is_defined())
		if(t == stream_duration() - 1) reached_end = true;
		
	if(reached_end) mark_end();
}

// TODO (all nodes): handle cross over end when ended input desactivated

	
bool sink_node::process_next_frame() {
	pull(current_time() + 1);
	return true;
}

void sink_node::seek(time_unit t) {
	MF_EXPECTS(is_seekable() && stream_duration_is_defined());
	if(t < 0 || t >= stream_duration()) throw std::invalid_argument("seek target time out of bounds");
	set_current_time(t - 1);
}


}}
