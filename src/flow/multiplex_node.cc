#include "multiplex_node.h"
#include "graph.h"
#include <vector>
#include <iostream>

#include <unistd.h>

namespace mf { namespace flow {

bool multiplex_node::process_next_frame() {	
	if(input_view_.is_null()) {
		set_current_time(0);
		input_.pull(0);
		input_view_.reset( input_.begin_read_frame(0) );
	} else if(next_pull_ != current_time()) {
		MF_DEBUG("pulling ", next_pull_.load(), "...");
		input_.end_read_frame(current_time());
		set_current_time(next_pull_);
		input_.pull(next_pull_);
		input_view_.reset( input_.begin_read_frame(next_pull_) );
	}
	return true;
}

void multiplex_node::thread_main_() {
	for(;;) process_next_frame();
}


void multiplex_node::pre_setup() {
	common_successor_ = &first_successor();

	time_unit min_off = minimal_offset_to(*common_successor_);
	time_unit max_off = maximal_offset_to(*common_successor_);
	
	input_.set_past_window(-min_off);
	input_.set_future_window(max_off);
}


void multiplex_node::setup() {
	for(auto&& base_out : outputs()) {
		base_out->define_frame_length(256);
	}
}


void multiplex_node::launch() {
	thread_ = std::move(std::thread(
		std::bind(&multiplex_node::thread_main_, this)
	));
}


void multiplex_node::stop() {
	thread_.join();
}



void multiplex_node_output::setup() {
}	


bool multiplex_node_output::pull(time_span span, bool reconnect) {
	multiplex_node& nd = (multiplex_node&)this_node();
	time_span exp_span(
		nd.common_successor_->current_time()+nd.minimal_offset_to(*nd.common_successor_),
		nd.common_successor_->current_time()+nd.maximal_offset_to(*nd.common_successor_)+1
	);
	
	nd.next_pull_ = nd.common_successor_->current_time();
		
	if(exp_span.includes(span)) {
		pull_time_ = span.start_time();
		return true;
	} else {
		return false;
	}
}


timed_frame_array_view multiplex_node_output::begin_read(time_unit duration) {
	multiplex_node& nd = (multiplex_node&)this_node();

	if(end_time() != -1 && pull_time_ + duration > end_time())
		duration = end_time() - pull_time_;
	
	while(nd.input_view_.is_null() ||
	! nd.input_view_.span().includes( time_span(pull_time_, pull_time_+duration+1) )) {
		usleep(100);
	}

	MF_ASSERT(nd.input_view_.span().includes( time_span(pull_time_, pull_time_+duration+1) ));
	
	time_span sp = nd.input_view_.span();
	
	return nd.input_view_(nd.input_view_.time_index(pull_time_), nd.input_view_.time_index(pull_time_+duration));
}


void multiplex_node_output::end_read(time_unit duration) {

}


time_unit multiplex_node_output::end_time() const {
	multiplex_node& nd = (multiplex_node&)this_node();
	return nd.input_.end_time();
}


frame_view multiplex_node_output::begin_write_frame(time_unit& t) {
	throw 0;
}


void multiplex_node_output::end_write_frame(bool was_last_frame) {
	throw 0;
}


void multiplex_node_output::cancel_write_frame() {
	throw 0;
}


}}


