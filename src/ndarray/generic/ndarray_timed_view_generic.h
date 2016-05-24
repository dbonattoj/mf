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

#ifndef MF_NDARRAY_TIMED_VIEW_GENERIC_H_
#define MF_NDARRAY_TIMED_VIEW_GENERIC_H_

#include "ndarray_view_generic.h"

namespace mf {

// TODO reduce verbosity

/// Generic \ref ndarray_timed_view where lower dimension(s) are type-erased.
template<std::size_t Dim>
class ndarray_timed_view_generic : public ndarray_timed_view<Dim + 1, byte> {
	using base = ndarray_timed_view<Dim + 1, byte>;

public:
	using shape_type = typename base::shape_type;
	using strides_type = typename base::strides_type;
	
private:
	frame_format format_;
	
public:
	ndarray_timed_view_generic() : format_(frame_format::null()) { }
	ndarray_timed_view_generic(const base& vw, const frame_format& format) :
		base(vw), format_(format) { }
	explicit ndarray_timed_view_generic(time_unit start_time, const frame_format& format) :
		base(start_time), format_(format) { }
	ndarray_timed_view_generic(const ndarray_view_generic<Dim>& gen_vw, time_unit start_time) :
		base(gen_vw, start_time), format_(gen_vw.format()) { }

	static ndarray_timed_view_generic null() { return ndarray_timed_view_generic(); }

	operator ndarray_view_generic<Dim> () const noexcept
		{ return ndarray_view_generic<Dim>(*this, format_); }
	
	const frame_format& format() const noexcept { return format_; }
	
	decltype(auto) slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const
		{ return ndarray_view_generic<Dim - 1>(base::slice(c, dimension), format_); }

	decltype(auto) operator[](std::ptrdiff_t c) const
		{ return ndarray_view_generic<Dim - 1>(base::operator[](c), format_); }

	decltype(auto) operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const
		{ return ndarray_timed_view_generic(base::operator()(start, end, step), format_); }
		
	decltype(auto) operator()(std::ptrdiff_t c) const
		{ return ndarray_timed_view_generic(base::operator()(c), format_); }

	decltype(auto) operator()() const
		{ return ndarray_timed_view_generic(base::operator()(), format_); }
};


template<std::size_t Generic_dim, std::size_t Concrete_dim, typename Concrete_elem>
ndarray_timed_view_generic<Generic_dim> to_generic(const ndarray_timed_view<Concrete_dim, Concrete_elem>& vw) {
	auto gen_vw = to_generic<Generic_dim>(static_cast<const ndarray_view<Concrete_dim, Concrete_elem>&>(vw));
	return ndarray_timed_view_generic<Generic_dim>(gen_vw, vw.start_time());
}



template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Generic_dim>
ndarray_timed_view<Concrete_dim, Concrete_elem> from_generic(
	const ndarray_timed_view_generic<Generic_dim>& gen_vw,
	const ndsize<Concrete_dim - Generic_dim>& frame_shape
) {
	auto vw = from_generic<Concrete_dim, Concrete_elem, Generic_dim>(static_cast<ndarray_view_generic<Generic_dim>>(gen_vw), frame_shape);
	return ndarray_timed_view<Concrete_dim, Concrete_elem>(vw, gen_vw.start_time());
}



}

#endif