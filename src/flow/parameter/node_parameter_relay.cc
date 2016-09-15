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

#include "node_parameter_relay.h"

namespace mf { namespace flow {

void node_parameter_relay::set_handler(node_parameter_id id, const handler_function_type& func) {
	handlers_.emplace(id, func);
}


bool node_parameter_relay::has_handler(node_parameter_id id) const {
	return (handlers_.find(id) != handlers_.end());
}


auto node_parameter_relay::handler(node_parameter_id id) const -> const handler_function_type& {
	return handlers_.at(id);
}


void node_parameter_relay::send_parameter(node_parameter_id id, const node_parameter_value& new_value) const {
	handler(id)(new_value);
}


}}
