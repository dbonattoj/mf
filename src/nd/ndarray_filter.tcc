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

#include "../utility/misc.h"
#include <iostream>

namespace mf {

template<std::size_t Dim, typename Elem, typename Kernel_elem>
kernel_placement<Dim, Elem, Kernel_elem> place_kernel_at(
	const ndarray_view<Dim, Elem>& view,
	const ndarray_view<Dim, Kernel_elem> kernel,
	const ndptrdiff<Dim>& pos
) {
	for(std::ptrdiff_t i = 0; i < Dim; ++i) Assert_crit(is_odd(kernel.shape()[i]));
	
	kernel_placement<Dim, Elem, Kernel_elem> placement;
	placement.absolute_position = pos;
	
	ndptrdiff<Dim> margin = kernel.shape() / 2; // gets rounded down. e.g. for (3,5) kernel --> (1,2)
	
	auto view_span = make_ndspan(pos - margin, pos + margin + 1);
	view_span = span_intersection(view.full_span(), view_span);
		
	auto kernel_span = make_ndspan(view_span.start_pos() - pos + margin, view_span.end_pos() - pos + margin);

	Assert_crit(kernel.full_span().includes(kernel_span));
	
	placement.view_section.reset(view.section(view_span));
	placement.kernel_section.reset(kernel.section(kernel_span));
	Assert_crit(placement.view_section.shape() == placement.kernel_section.shape());
	
	placement.section_position = pos - view_span.start_pos();
	Assert_crit(placement.view_section.full_span().includes(placement.section_position));
	Assert_crit(&placement.view_section.at(placement.section_position) == &view.at(pos));

	return placement;
}


template<std::size_t Dim, typename In_elem, typename Out_elem, typename Kernel_elem, typename Function>
void apply_kernel(
	Function&& func,
	const ndarray_view<Dim, In_elem>& in_view,
	const ndarray_view<Dim, Out_elem>& out_view,
	const ndarray_view<Dim, Kernel_elem> kernel
) {
	Assert_crit(in_view.shape() == out_view.shape());
	
	for(auto it = out_view.begin(); it != out_view.end(); ++it) {
		auto pos = it.coordinates();
		auto placement = place_kernel_at(in_view, kernel, pos);
		func(placement, *it);
	}
}


}
