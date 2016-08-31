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

#ifndef MF_NDARRAY_TIMED_VIEW_OPAQUE_H_
#define MF_NDARRAY_TIMED_VIEW_OPAQUE_H_

#include "ndarray_view_opaque.h"
#include "../detail/ndarray_timed_view_derived.h"

namespace mf {


/// \ref ndarray_view_opaque with absolute time indices associated to first (opaque) dimension.
template<std::size_t Dim, typename Format = opaque_format_array, bool Mutable = true>
using ndarray_timed_view_opaque = detail::ndarray_timed_view_derived<ndarray_view_opaque<Dim, Format, Mutable>>;


}

#endif
