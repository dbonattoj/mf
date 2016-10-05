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
}


node::pull_result sink_node::pull(time_unit t) {
	// Set time and create job
	set_current_time_(t);
	processing_node_job job(*this, std::move(current_parameter_valuation_())); // reads current time of node

	// Let handler pre-process frame
	handler_result handler_res = handler_pre_process_(job);
	if(handler_res == handler_result::failure) return pull_result::fatal_failure;
	else if(handler_res == handler_result::end_of_stream) return pull_result::end_of_stream;

	// Pre-pull the activated inputs
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(in.is_activated()) in.pre_pull();
	}
	
	// Pull the activated inputs
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(! in.is_activated()) continue;
		
		pull_result res = in.pull();
		if(res == pull_result::transitory_failure) throw sequencing_error("sink received transitional failure");
		else if(res != pull_result::success) return res;
	}

	// Begin reading from the activated inputs (in job object)
	// If error occurs, job object will cancel read during stack unwinding
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(! in.is_activated()) continue;
		bool began = job.begin_input(in);
		if(! began) throw sequencing_error("sink received transitional failure");
	}

	// Let handler process frame
	handler_res = handler_process_(job);
	if(handler_res == handler_result::failure) return pull_result::fatal_failure;
	else if(handler_res == handler_result::end_of_stream) return pull_result::end_of_stream;

	// End reading from the inputs 
	job.end_inputs();

	// Finish the job
	update_parameters_(job.parameters()); // TODO move instead of copy

	return pull_result::success;
}

	
node::pull_result sink_node::pull_next_frame() {
	return pull(current_time() + 1);
}

void sink_node::seek(time_unit t) {
	set_current_time_(t - 1);
}


}}
