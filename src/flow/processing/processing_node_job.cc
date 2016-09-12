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
#include <utility>

namespace mf { namespace flow {

processing_node_job::processing_node_job(processing_node& nd, const node_parameter_valuation& params) :
	node_(nd),
	node_parameters_(params),
	input_views_(nd.inputs_count()) { }


processing_node_job::processing_node_job(processing_node& nd, node_parameter_valuation&& params) :
	node_(nd),
	node_parameters_(std::move(params)),
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


bool processing_node_job::has_input_view(input_index_type index) const noexcept {
	return ! input_views_.at(index).is_null();
}


const node_frame_window_view& processing_node_job::input_view(input_index_type index) const {
	Expects(has_input_view(index));
	return input_views_.at(index);
}


bool processing_node_job::has_output_view() const noexcept {
	return ! output_view_.is_null();
}


const node_frame_view& processing_node_job::output_view() const {
	Expects(has_output_view());
	return output_view_;
}


bool processing_node_job::has_parameter(parameter_id id) const {
	return node_parameters_.has(id);
}


node_parameter_value& processing_node_job::parameter(parameter_id id) {
	return node_parameters_(id);
}


const node_parameter_value& processing_node_job::parameter(parameter_id id) const {
	return node_parameters_(id);
}


const node_parameter_valuation& processing_node_job::parameters() const {
	return node_parameters_;
}


bool processing_node_job::has_input_parameter(parameter_id id, time_unit t) {
	auto possible_inputs = node_.propagated_parameters_inputs(id);
	for(input_index_type input_index : possible_inputs) {
		if(! has_input_view(input_index)) continue;
		const node_frame_window_view& vw = input_view(input_index);
		if(t < vw.start_time() || t >= vw.end_time()) continue;
		const node_parameter_valuation& input_parameter_valuation = vw.at_time(t).propagated_parameters();
		if(input_parameter_valuation.has(id)) return true;
	}
	return false;
}


const node_parameter_value& processing_node_job::input_parameter(parameter_id id, time_unit t) {
	auto possible_inputs = node_.propagated_parameters_inputs(id);
	for(input_index_type input_index : possible_inputs) {
		if(! has_input_view(input_index)) continue;
		const node_frame_window_view& vw = input_view(input_index);
		if(t < vw.start_time() || t >= vw.end_time()) continue;
		const node_parameter_valuation& input_parameter_valuation = vw.at_time(t).propagated_parameters();
		if(input_parameter_valuation.has(id)) return input_parameter_valuation(id);
	}
	throw std::logic_error("input parameter not present");
}



}}
