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

#include "processing_node_job.h"

namespace mf { namespace flow {

processing_node_job::processing_node_job(processing_node& nd) :
	node_(nd),
	input_views_(nd.inputs_count()) { }


processing_node_job::~processing_node_job() {
	Expects(output_view_.is_null(), "processing_node_job must be detached prior to destruction");
	cancel_inputs();
}


bool processing_node_job::begin_input(processing_node_input& in) {
	std::ptrdiff_t index = in.index();
	timed_frame_array_view vw = in.begin_read_frame();
	if(vw.is_null()) return false;
	input_views_.at(index).reset(vw);
	return true;
}


void processing_node_job::end_input(processing_node_input& in) {
	std::ptrdiff_t index = in.index();
	in.end_read_frame();	
	input_views_.at(index).reset();
}


void processing_node_job::cancel_inputs() {
	for(std::ptrdiff_t index = 0; index < input_views_.size(); ++index) {
		timed_frame_array_view& vw = input_views_.at(index);
		if(! vw.is_null()) {
			node_.input_at(index).cancel_read_frame();
			vw.reset();
		}
	}
}


void processing_node_job::attach_output_view(const frame_view& out_vw) {
	output_view_.reset(out_vw);
}


void processing_node_job::detach_output_view() {
	output_view_.reset();
}


bool processing_node_job::has_input_view(std::ptrdiff_t index) const noexcept {
	return ! input_views_.at(index).is_null();
}


const timed_frame_array_view& processing_node_job::input_view(std::ptrdiff_t index) const {
	Expects(has_input_view(index));
	return input_views_.at(index);
}


bool processing_node_job::has_output_view() const noexcept {
	return ! output_view_.is_null();
}


const frame_view& processing_node_job::output_view() const {
	Expects(has_output_view());
	return output_view_;
}

}}
