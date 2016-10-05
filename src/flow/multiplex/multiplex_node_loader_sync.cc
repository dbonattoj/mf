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

#include "multiplex_node_loader_sync.h"
#include "../node_graph.h"
#include <utility>
#include <functional>

namespace mf { namespace flow {

multiplex_node::sync_loader::sync_loader(multiplex_node& nd) :
	loader(nd, nd.output_at(0).reader_thread_index())
{
	Assert(! nd.outputs_on_different_threads_(),
		"multiplex_node with sync_loader must have all outputs on same reader thread");
}


void multiplex_node::sync_loader::pre_stop() { }


void multiplex_node::sync_loader::stop() { }


void multiplex_node::sync_loader::launch() { }


void multiplex_node::sync_loader::pre_pull(time_span span) { }


node::pull_result multiplex_node::sync_loader::pull(time_span& span) {
	// get expected input span, based on current time of the common successor node
	time_unit successor_time = this_node().capture_fcs_time_();
	time_span expected_input_span = this_node().expected_input_span_(successor_time);
	
	if(! expected_input_span.includes(span)) {
		// tfail if pulled span not in this expected span
		span.set_end_time(span.start_time());
		return pull_result::transitory_failure;
	}
	
	if((successor_time != this_node().current_time()) || (input_pull_result_ != pull_result::success)) {
		// reload input view if successor_time has changed since last time
		// with successor_time as captured before
		input_pull_result_ = this_node().load_input_view_(successor_time);
		Assert(this_node().current_time() == successor_time);
	}

	// get portion of the requested span that is not loaded
	time_span loaded_input_span = this_node().loaded_input_span_();
	span = span_intersection(span, loaded_input_span);


	if(input_pull_result_ != pull_result::success) return input_pull_result_;
	else if(loaded_input_span.includes(span)) return pull_result::success;
	else return pull_result::end_of_stream;
}


node_frame_window_view multiplex_node::sync_loader::begin_read(time_span span) {
	timed_frame_array_view input_view = this_node().loaded_input_view_();
	if(input_view.span().includes(span)) return input_view.time_section(span);
	else return node_frame_window_view::null();
}


void multiplex_node::sync_loader::end_read(time_unit duration) { }


}}
