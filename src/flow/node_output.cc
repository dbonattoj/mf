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

#include "node_output.h"
#include "node_input.h"

namespace mf { namespace flow {

node_output::node_output(node& nd) :
	node_(nd) { }


thread_index node_output::reader_thread_index() const {
	Assert(is_connected(), "output reader thread index known only when connected to an input");
	return connected_input().reader_thread_index();
}


bool node_output::add_relayed_parameter_if_needed(node_parameter_id id, const node_parameter_relay& preceding_relay) {
	Assert(is_connected());
	return connected_node().add_relayed_parameter_if_needed(id, preceding_relay);
}


bool node_output::add_propagated_parameter_if_needed(node_parameter_id id) {
	std::cout << "node_output(" << this_node().name() << ")::add_propagated_parameter_if_needed(" << id << ")" << std::endl;

	Assert(is_connected());
	if(has_propagated_parameter(id)) return true;
	bool needed = connected_node().add_propagated_parameter_if_needed(id);
	if(needed) propagated_parameters_.push_back(id);
	return needed;
}


bool node_output::has_propagated_parameter(node_parameter_id id) const {
	auto it = std::find(propagated_parameters_.cbegin(), propagated_parameters_.cend(), id);
	return (it != propagated_parameters_.cend());
}


void node_output::input_has_connected(node_input& input) {
	connected_input_ = &input;
}


void node_output::input_has_disconnected() {
	connected_input_ = nullptr;
}


bool node_output::is_online() const {
	if(connected_input_->is_activated() == false) return false;
	else return (connected_input_->this_node().state() == node::online);
}


node& node_output::connected_node() const noexcept {
	return connected_input_->this_node();
}


time_unit node_output::end_time() const noexcept {
	return this_node().end_time();
}

}}
