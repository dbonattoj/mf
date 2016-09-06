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

#ifndef MF_FLOW_NODE_PARAMETER_H_
#define MF_FLOW_NODE_PARAMETER_H_

#include <string>
#include <memory>
#include "node_parameter_holder.h"

namespace mf { namespace flow {

using node_parameter_id = int;

enum class node_parameter_kind {
	deterministic,
	dynamic
};


/// Node parameter with its current value.
class node_parameter {
private:
	node_parameter_id id_;
	node_parameter_kind kind_;

public:
	node_parameter(node_parameter_id id, node_parameter_kind kind);
	
	node_parameter_id id() const { return id_; }
	node_parameter_kind kind() const { return kind_; }
};


}}

#endif
