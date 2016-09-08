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

#ifndef MF_FLOW_PROCESSING_NODE_JOB_H_
#define MF_FLOW_PROCESSING_NODE_JOB_H_

#include "processing_node.h"
#include "../parameter/node_parameter_valuation.h"
#include <vector>

namespace mf { namespace flow {

/// Job of \ref processing_node.
/** Created temporarily during processing of one frame by the \ref processing_node.
 ** Contains readable view to frames from each activated input, and writable multi-channel view for output frame.
 ** Passed to the \ref processing_node_handler.
 ** begin_input() opens the input view, and calls node_input::begin_read_frame(). Input have that have not been closed
 ** with end_input() are automatically closed and cancelled using node_input::cancel_read_frame().
 ** The \ref processing_node_job acts as a resource handle to the input views.
 ** However the output view must be opened and closed by the caller, and attached to the \ref processing_node_job.
 ** \ref processing_node_job must not be destructed with the output view still attached. */
class processing_node_job {
private:
	processing_node& node_;
	const node_parameter_valuation& node_parameters_;
	std::vector<timed_frame_array_view> input_views_;
	frame_view output_view_;

	bool end_marked_ = false;
	
public:
	processing_node_job(processing_node& nd, const node_parameter_valuation& params);
	processing_node_job(const processing_node_job&) = delete;
	processing_node_job(processing_node_job&&) = default;
	~processing_node_job();

	processing_node_job& operator=(const processing_node_job&) = delete;
	processing_node_job& operator=(processing_node_job&&) = default;
	
	void attach_output_view(const frame_view&);
	void detach_output_view();

	bool begin_input(processing_node_input&);
	void end_input(processing_node_input&);
	void cancel_inputs();
	
	bool end_was_marked() const noexcept { return end_marked_; }
		
	time_unit time() const noexcept { return node_.current_time(); }
	void mark_end() noexcept { end_marked_ = true; }
	
	bool has_input_view(std::ptrdiff_t index) const noexcept;
	const timed_frame_array_view& input_view(std::ptrdiff_t index) const;
	
	bool has_output_view() const noexcept;
	const frame_view& output_view() const;
};

}}

#endif
