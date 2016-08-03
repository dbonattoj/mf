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

#ifndef MF_NDARRAY_GENERIC_H_
#define MF_NDARRAY_GENERIC_H_

#include "frame_format.h"
#include "ndarray_view_generic.h"
#include "../ndarray.h"
#include "../../elem.h"
#include "../../os/memory.h"

namespace mf {

/// Generic \ref ndarray where lower dimension(s) are type-erased.
/** Analogous to \ref ndarray_view_generic. */
template<std::size_t Dim, typename Allocator = raw_allocator>
class ndarray_generic : public ndarray<Dim + 1, byte, Allocator> {
	using base = ndarray<Dim + 1, byte, Allocator>;

public:
	using view_type = ndarray_view_generic<Dim>;

	using generic_shape_type = typename view_type::generic_shape_type;
	using generic_strides_type = typename view_type::generic_strides_type;

private:
	frame_format format_;

public:
	ndarray_generic
	(const frame_format&, const generic_shape_type&, std::size_t padding = 0, const Allocator& = Allocator());
	
	ndarray_generic(const ndarray_generic&) = default;

	const frame_format& format() const noexcept { return format_; }

	generic_shape_type generic_shape() const { return base::shape().head(); }
	generic_strides_type generic_strides() const { return base::strides().head(); }

	view_type view() { return view_type(format_, base::view()); }	
};


}

#include "ndarray_generic.tcc"

#endif
