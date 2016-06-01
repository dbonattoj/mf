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

#include "node_job.h"

namespace mf { namespace flow {

node_job::node_job(node& nd) :
	node_(nd),
	inputs_map_(nd.inputs().size(), nullptr),
	outputs_map_(nd.outputs().size(), nullptr)
{
	inputs_stack_.reserve(nd.inputs().size());
	outputs_stack_.reserve(nd.outputs().size());
}


node_job::~node_job() {
	MF_ASSERT(inputs_stack_.empty());
	MF_ASSERT(outputs_stack_.empty());
}
	

void node_job::push_input(node_input& in, const timed_frame_array_view& vw) {
	MF_EXPECTS(! vw.is_null());
	inputs_stack_.emplace_back(&in, vw);
	inputs_map_.at(in.index()) = &inputs_stack_.back();
}


void node_job::push_output(node_output& out, const frame_view& vw) {
	MF_EXPECTS(! vw.is_null());
	outputs_stack_.emplace_back(&out, vw);
	outputs_map_.at(out.index()) = &outputs_stack_.back();
}


node_input* node_job::pop_input() {
	if(inputs_stack_.empty()) return nullptr;
	node_input* input = inputs_stack_.back().first;
	inputs_stack_.pop_back();
	return input;
}


node_output* node_job::pop_output() {
	if(outputs_stack_.empty()) return nullptr;
	node_output* output = outputs_stack_.back().first;
	outputs_stack_.pop_back();
	return output;
}
	
	
void node_job::define_time(time_unit t) {
	time_ = t;
}

}}
