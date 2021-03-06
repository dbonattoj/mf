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

#ifndef MF_FLOW_NODE_STREAM_PROPERTIES_H_
#define MF_FLOW_NODE_STREAM_PROPERTIES_H_

#include "../common.h"
#include <stdexcept>

namespace mf {

class node_stream_properties {
public:
	enum policy_type { undefined, seekable, forward };

private:
	policy_type policy_ = undefined;
	time_unit duration_ = -1;

public:
	node_stream_properties() = default;

	explicit node_stream_properties(policy_type pol, time_unit dur = -1) :
	policy_(pol), duration_(dur) {
		if(pol == seekable && dur == -1) throw std::invalid_argument("duration of seekable stream must be defined");
	}
	
	node_stream_properties(const node_stream_properties&) = default;
	node_stream_properties& operator=(const node_stream_properties&) = default;

	policy_type policy() const noexcept { return policy_; }
	time_unit duration() const noexcept { return duration_; }

	bool duration_is_defined() const noexcept { return (duration_ != -1); }
	
	bool is_seekable() const noexcept { return (policy_ == seekable); }
};

}

#endif
