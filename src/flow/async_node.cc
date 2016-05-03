#include "async_node.h"
#include "graph.h"

namespace mf { namespace flow {

bool async_node::frame_() {
	node_job job = make_job();
	time_unit t;
	
	// begin writing frame to output
	// output determines time t of frame to write
	node_output& out = outputs().front();
	auto out_view = out.begin_write_frame(t);
	if(out_view.is_null()) return false; // stopped
	
	set_current_time(t);
	
	// with time defined, run preprocess
	// concrete node may activate/desactivate inputs now
	this->pre_process(t);

	// add output view to job
	job.define_time(t);
	job.push_output(out, out_view);
	
	// pull & begin reading from activated inputs
	bool stopped = false;
	for(node_input& in : inputs()) {
		MF_ASSERT_MSG(! in.reached_end(t), "input of node must not already be at end");

		if(in.is_activated()) {
			// pull frame t from input
			// for sequential node: frame is now processed
			in.pull(t);
			
			// begin reading frame 
			timed_frames_view in_view = in.begin_read_frame(t);
			MF_ASSERT(in_view.span().includes(t));
			if(in_view.is_null()) { stopped = true; break; }
			
			// add to job
			job.push_input(in, in_view);
		}
	}
	// TODO split pull/begin_read, and begin_read (wait) simultaneously if possible (async_node inputs)
	
	if(stopped) {
		// stopped while reading from input:
		// need to cancel output and already-opened inputs, and then quit
		cancel_job(job);
		return false;
	}
	
	// process frame in concrete subclass
	this->process(job);
	
	// check if node reached end
	// determined by stream duration or reached_end() for source
	// when stream duration undefined, end is reached when an input reached it
	bool reached_end = false;
	if(stream_duration_is_defined()) {
		if(t == stream_duration() - 1) reached_end = true;
		MF_ASSERT(t < stream_duration());
	} else if(is_source()) {
		reached_end = job.end_was_marked();
	}

	// end reading from inputs
	while(node_input* in = job.pop_input()) {
		in->end_read_frame(t);
		if(in->reached_end(t)) reached_end = true;
	}
	
	// end writing to output,
	// indicate if last frame was reached
	job.pop_output()->end_write_frame(reached_end);
	
	if(reached_end) mark_end();
		
	return true;
}


void async_node::thread_main_() {
	bool continuing = true;
	while(continuing) {
		continuing = frame_();
	}
}


async_node::async_node(graph& gr) : node(gr) { }


async_node::~async_node() {
	MF_EXPECTS_MSG(!running_, "async_node must have been stopped prior to destruction");
}


void async_node::internal_setup() {
	if(outputs().size() != 1) throw invalid_flow_graph("async_node must have exactly 1 output");
	this->setup();
}
	

void async_node::launch() {
	MF_EXPECTS(! running_);
	thread_ = std::move(std::thread(
		std::bind(&async_node::thread_main_, this)
	));
	running_ = true;
}


void async_node::stop() {
	MF_EXPECTS(running_);
	MF_EXPECTS(thread_.joinable());
	thread_.join();
	running_ = false;
}


void async_node_output::setup() {
	node& connected_node = connected_input().this_node();
	
	time_unit offset_diff = this_node().offset() - connected_node.offset();
	time_unit required_capacity = 1 + connected_input().past_window_duration() + offset_diff;


	frame_array_properties prop(format(), frame_length(), required_capacity);
	ring_.reset(new shared_ring(prop, this_node().is_seekable(), this_node().stream_duration()));
}	


void async_node_output::pull(time_span span) {
	time_unit t = span.start_time();
	time_unit ring_read_t = ring_->read_start_time();
	if(t != ring_read_t) {
		if(ring_->is_seekable()) ring_->seek(t);
		else if(t > ring_read_t) ring_->skip(ring_read_t - t);
		else throw std::logic_error("ring not seekable but async_node output attempted to seek to past");
	}	
}


timed_frames_view async_node_output::begin_read(time_unit duration) {
	event& stop_event = this_node().this_graph().stop_event();

	timed_frames_view view;
	bool got_view = false;
	do {
		bool status = ring_->wait_readable(stop_event);
		if(! status) break;
		
		got_view = ring_->try_begin_read(duration, view); // TODO try_begin_read: return view? instead
	} while(! got_view);
	
	return view;
}


void async_node_output::end_read(time_unit duration) {
	ring_->end_read(duration);
}


time_unit async_node_output::end_time() const {
	return ring_->end_time();
}


frame_view async_node_output::begin_write_frame(time_unit& t) {
	event& stop_event = this_node().this_graph().stop_event();

	timed_frames_view view;
	bool got_view = false;
	
	do {
		bool status = ring_->wait_writable(stop_event);
		if(! status) return frame_view::null();
		
		got_view = ring_->try_begin_write(1, view);
	} while(! got_view);
	
	MF_ASSERT(view.duration() == 1);
	t = view.start_time();
	return view[0];
}


void async_node_output::end_write_frame(bool was_last_frame) {
	bool mark_end = was_last_frame && !ring_->is_seekable();
	ring_->end_write(1, mark_end);
}


void async_node_output::cancel_write_frame() {
	ring_->end_write(0, false);
}

}}
