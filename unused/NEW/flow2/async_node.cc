#include "async_node.h"

namespace mf { namespace flow {

bool async_node::frame_() {

}


void async_node::thread_main_() {
	bool continuing = true;
	while(continuing) {
		continuing = frame_();
	}
}


async_node::async_node() { }


async_node::~async_node() {
	MF_EXPECTS_MSG(!running_, "async_node must have been stopped prior to destruction");
}


void async_node::internal_setup() {
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
	stop_event_.notify();
	thread_.join();
	running_ = false;
}


void async_node::pull(time_unit t) {
	
}


void async_node_output::setup() {
	node_input& connected_input = connected_input();
	node& connected_node = connected_input.this_node();
	
	time_unit offset_diff = connected_node.offset() - this_node().offset();
	required_ring_capacity_ = 1 + connected_input.past_window_duration() + offset_diff;
}


void async_node_output::pull(time_unit t) {

}


timed_frames_view async_node_output::begin_read(time_unit duration) {

}


void async_node_output::end_read(time_unit duration) {

}


time_unit async_node_output::end_time() const {

}


frame_view async_node_output::begin_write_frame(time_unit& t) {

}


void async_node_output::end_write_frame(bool was_last_frame) {

}


}}
