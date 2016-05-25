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

#ifndef MF_FLOW_CONVERT_FILTER_H_
#define MF_FLOW_CONVERT_FILTER_H_

#include "simple_filter.h"
#include <algorithm>

namespace mf { namespace flow {

template<std::size_t Dim, typename Input_elem, typename Output_elem, typename Element_function>
class convert_filter : public simple_filter<Dim, Input_elem, Output_elem> {
	using base = simple_filter<Dim, Input_elem, Output_elem>;

	using input_view_type = typename base::input_view_type;
	using output_view_type = typename base::output_view_type;
	using job_type = typename base::job_type;
	
private:
	Element_function function_;

protected:
	void process_frame(const input_view_type& in, const output_view_type& out, job_type&) override {
		std::transform(in.begin(), in.end(), out.begin(), function_);
	}

public:
	convert_filter(flow::filter_node& nd, Element_function func) :
		base(nd), function_(func) { }
};

}}

#endif
