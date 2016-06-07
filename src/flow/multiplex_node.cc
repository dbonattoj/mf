#include "multiplex_node.h"
#include "graph.h"
#include <vector>
#include <iostream>

#include <unistd.h>

namespace mf { namespace flow {
/*
multiplex_node::output_rings_vector_type multiplex_node::output_rings_() {
	output_rings_vector_type rings;
	for(auto&& base_out : outputs()) {
		multiplex_node_output& out = dynamic_cast<multiplex_node_output&>(*base_out);
		rings.emplace_back(out.ring());
	}
	return rings;
}
*/

bool multiplex_node::process_next_frame() {
	usleep(10000);
	
	if(input_view_.is_null()) {
		input_.pull(0);
		set_current_time(0);
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


void multiplex_node_output::pull(time_span span, bool reactivate) {
	multiplex_node& nd = (multiplex_node&)this_node();
	time_span exp_span(
		nd.common_successor_->current_time()+nd.minimal_offset_to(*nd.common_successor_),
		nd.common_successor_->current_time()+nd.maximal_offset_to(*nd.common_successor_)+1
	);
	
	nd.next_pull_ = nd.common_successor_->current_time();
	
	MF_DEBUG(exp_span.includes(span), " = ", span, " in? ", exp_span);
	
	MF_ASSERT(exp_span.includes(span));
		
	pull_time_ = span.start_time();
}


timed_frame_array_view multiplex_node_output::begin_read(time_unit duration) {
	multiplex_node& nd = (multiplex_node&)this_node();

	while(nd.input_view_.is_null() ||
	! nd.input_view_.span().includes( time_span(pull_time_, pull_time_+duration+1) )) {
		usleep(100);
	}
	
	MF_DEBUG("begin_read... ", nd.input_view_.span());
	MF_ASSERT(nd.input_view_.span().includes( time_span(pull_time_, pull_time_+duration+1) ));
	
	time_span sp = nd.input_view_.span();
	
	return nd.input_view_(nd.input_view_.time_index(pull_time_), nd.input_view_.time_index(pull_time_+duration));
}


void multiplex_node_output::end_read(time_unit duration) {

}


time_unit multiplex_node_output::end_time() const {
	return 100;
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


