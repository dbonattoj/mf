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

#include "result_improve.h"
#include <mf/image/image.h>
#include <mf/opencv.h>
#include <mf/ndarray/ndarray_filter.h>

using namespace mf;


void result_improve_filter::setup() {
	output.define_frame_shape(input.frame_shape());
}

void result_improve_filter::process(mf::flow::node_job& job) {
	job.out(output) = job.in(input); 
	
	auto in = job.in(input);
	auto in_full = job.in_full(input);
	auto out = job.out(output);
	
	ndarray<3, bool> kernel(make_ndsize(5, 3, 3));
	std::fill(kernel.begin(), kernel.end(), true);
	
	for(auto it = in.begin(); it != in.end(); ++it) {
		auto coord = it.coordinates();
		if(it->is_null()) {
			auto p = place_kernel_at(in_full, kernel.view(), ndcoord_cat(in_full.time_index(job.time()), coord));
			real r = 0, g = 0, b = 0, c = 0;
			for(const auto& col : p.view_section) {
				if(col.is_null()) continue;
				r += col.elem.r; g += col.elem.g; b += col.elem.b; c += 1.0;
			}
			if(c == 0) out.at(coord) = mf::masked_elem<mf::rgb_color>::null();
			else out.at(coord) = rgb_color( clamp(r/c, 0.0, 255.0), clamp(g/c, 0.0, 255.0), clamp(b/c, 0.0, 255.0) );
		} else {
			out.at(coord) = *it;
		}
	}
}
