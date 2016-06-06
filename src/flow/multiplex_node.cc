#include "multiplex_node.h"
#include "graph.h"
#include <vector>
#include <iostream>

#include <unistd.h>

namespace mf { namespace flow {

multiplex_node::output_rings_vector_type multiplex_node::output_rings_() {
	output_rings_vector_type rings;
	for(auto&& base_out : outputs()) {
		multiplex_node_output& out = dynamic_cast<multiplex_node_output&>(*base_out);
		rings.emplace_back(out.ring());
	}
	return rings;
}


bool multiplex_node::process_next_frame() {
	output_rings_vector_type output_rings = output_rings_();
	
	auto it = shared_ring::wait_any_writable(output_rings.begin(), output_rings.end(), this_graph().stop_event());
		
	if(it != output_rings.end()) {
		shared_ring& rng = *it;
		auto vw = rng.try_begin_write(1);

		auto prefix = (&rng != &(output_rings.front().get())) ? "                             " : " ";
 
		if(vw) {
			time_unit t = vw.start_time();
			
			time_unit old_t = current_time();
			set_current_time(t);
			
			if(input_view_.is_null()) {
				input_.pull(t);
				input_view_.reset(input_.begin_read_frame(t));
			} else if(! input_view_.span().includes(t)) {
				input_.end_read_frame(old_t);
				input_.pull(t);
				input_view_.reset(input_.begin_read_frame(t));
			}
			
			
			MF_DEBUG(t, " <<<in?<<< ", input_view_.span());
			
			//if(t >= input_view_.end_time()) {
			//	input_->end_read_frame(t);
			//	input_view_.reset( input_->begin_read_frame(0) );
			//	MF_DEBUG("... ", t, " <<<in?<<< ", input_view_.span());
			//}
			
			rng.end_write(1);
			
			std::cout << prefix << "wrote " << vw.start_time() << std::endl;
		} else {
			std::cout << prefix << "not wrote" << std::endl;
		}
	}
				usleep(100000);
		
	return true;
}

void multiplex_node::thread_main_() {
	const node& suc = first_successor();
	time_unit min_off = minimal_offset_to(suc);
	time_unit max_off = maximal_offset_to(suc);

	for(;;) process_next_frame();
}


void multiplex_node::pre_setup() {
	const node& suc = first_successor();
	time_unit min_off = minimal_offset_to(suc);
	time_unit max_off = maximal_offset_to(suc);

	MF_DEBUG_EXPR(min_off, max_off, max_off-min_off);	

	input_.set_past_window(-min_off + 1);
	input_.set_future_window(max_off + 1);
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
	node& connected_node = connected_input().this_node();

	time_unit required_capacity = 1 + this_node().maximal_offset_to(connected_node) - this_node().minimal_offset_to(connected_node);

	ndarray_generic_properties prop(format(), frame_length(), required_capacity);
	ring_.reset(new shared_ring(prop, this_node().is_seekable(), this_node().stream_duration()));
}	


void multiplex_node_output::pull(time_span span, bool reactivate) {
	time_unit t = span.start_time();
	time_unit ring_read_t = ring_->read_start_time();
	if(t != ring_read_t) {
		if(ring_->is_seekable()) ring_->seek(t);
		else if(t > ring_read_t) ring_->skip(t - ring_read_t);
		else throw std::logic_error("ring not seekable but async_node output attempted to seek to past");
	}
}


timed_frame_array_view multiplex_node_output::begin_read(time_unit duration) {
	event& stop_event = this_node().this_graph().stop_event();
	
	for(;;) {
		bool status = ring_->wait_readable(stop_event);
		if(! status) break;
		
		timed_frame_array_view view = ring_->try_begin_read(duration);
		if(view.is_null()) continue;
		
		return view;
	}
	
	return timed_frame_array_view::null();
}


void multiplex_node_output::end_read(time_unit duration) {
	ring_->end_read(duration);
}


time_unit multiplex_node_output::end_time() const {
	return ring_->end_time();
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


