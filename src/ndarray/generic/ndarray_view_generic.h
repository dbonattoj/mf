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

#ifndef MF_NDARRAY_VIEW_GENERIC_H_
#define MF_NDARRAY_VIEW_GENERIC_H_

#include "frame_format.h"
#include "../ndarray_view.h"
#include "../ndarray_timed_view.h"
#include "../../common.h"
#include <stdexcept>

namespace mf {

/// Generic \ref ndarray_view where lower dimension(s) are type-erased.
/** `ndarray_view_generic<Dim>` is a type-erased view of `ndarray_view<Dim + K, Elem>`, whose lower `K` dimensions must
 ** have default strides (possibly with padding). Elements of `ndarray_view_generic<Dim>` correspond to sections of
 ** `ndarray_view<Dim + K, Elem>` which fix these `K` dimensions, and are called *frames*.
 ** The view is no longer templatized for the dimensionality and element type of the frames. Frame format information
 ** is stored at runtime as \ref frame_format object.
 ** 
 ** Alternately, frames can be *composite*, i.e. contain data from multiple ndarrays of differing dimension and element
 ** types.
 ** 
 ** `ndarray_view_generic<Dim>` derives from `ndarray_view<Dim + 1, byte>`. Lowest dimension has stride 1, and length
 ** equal to frame length in bytes. The stride of the next dimension is set to respect the frame alignment requirement.
 ** For example with `ndarray_view_generic<2> arr`, the expression `arr[1][2][123]` is the 123th byte of frame $(1,2)$.
 ** 
 ** `Dim` is the *generic dimension*, and `Dim + K` the *concrete dimension*. `K` is the *frame dimension*.
 ** Along the generic dimensions, sectionning and slicing operations can be used and return another (lower-dimensional)
 ** `ndarray_view_generic` with the same frame format information. Conversion to and from concrete `ndarray_view` is
 ** done with the functions \ref to_generic() and \ref from_generic(). */
template<std::size_t Dim>
class ndarray_view_generic : public ndarray_view<Dim + 1, byte> {	
	using base = ndarray_view<Dim + 1, byte>;

public:	
	using generic_shape_type = ndsize<Dim>;
	using generic_strides_type = ndptrdiff<Dim>;
	
private:
	frame_format format_;

protected:
	ndarray_view_generic section_(std::ptrdiff_t dim, std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const {
		Expects(dim < Dim);
		return ndarray_view_generic(format_, base::section_(dim, start, end, step));
	}


public:
	static ndarray_view_generic null() { return ndarray_view_generic(); }
	ndarray_view_generic() = default;

	ndarray_view_generic(const frame_format& frm, const base& vw) :
		base(vw), format_(frm) { }
	
	ndarray_view_generic(const frame_format&, byte* start, const generic_shape_type&, const generic_strides_type&);

	generic_shape_type generic_shape() const { return base::shape().head(); }
	generic_strides_type generic_strides() const { return base::strides().head(); }

	void reset(const ndarray_view_generic& other) noexcept {
		base::reset(other);
		format_ = other.format_;
	}
	void reset() noexcept { reset(null()); }
	
	const frame_format& format() const noexcept { return format_; }

	ndarray_view_generic array_at(std::ptrdiff_t array_index) const;
	
	decltype(auto) slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const
		{ return ndarray_view_generic<Dim - 1>(format_, base::slice(c, dimension)); }

	decltype(auto) operator[](std::ptrdiff_t c) const
		{ return ndarray_view_generic<Dim - 1>(format_, base::operator[](c)); }

	decltype(auto) operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const
		{ return ndarray_view_generic(format_, base::operator()(start, end, step)); }
		
	decltype(auto) operator()(std::ptrdiff_t c) const
		{ return ndarray_view_generic(format_, base::operator()(c)); }

	decltype(auto) operator()() const
		{ return ndarray_view_generic(format_, base::operator()()); }
};



/// Cast input `ndarray_view` to generic `ndarray_view_generic` with given dimension.
template<std::size_t Generic_dim, std::size_t Concrete_dim, typename Concrete_elem>
ndarray_view_generic<Generic_dim> to_generic(const ndarray_view<Concrete_dim, Concrete_elem>& vw);



/// Cast input `ndarray_view_generic` to concrete `ndarray_view` with given dimension, frame shape and element type.
template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Generic_dim>
ndarray_view<Concrete_dim, Concrete_elem> from_generic(
	const ndarray_view_generic<Generic_dim>& vw,
	const ndsize<Concrete_dim - Generic_dim>& frame_shape,
	std::ptrdiff_t array_index = 0
);



}


#include "ndarray_view_generic.tcc"

#endif
