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
	//MF_ASSERT(inputs_stack_.empty());
	//MF_ASSERT(outputs_stack_.empty());
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
