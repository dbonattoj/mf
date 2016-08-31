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

#ifndef MF_TESTSUPPORT_NDARRAY_OPAQUE_H_
#define MF_TESTSUPPORT_NDARRAY_OPAQUE_H_

#include <mf/nd/opaque/ndarray_opaque.h>
#include <mf/nd/opaque/opaque_format_multi_array.h>
#include <sstream>
#include <vector>

namespace mf { namespace test {

template<std::size_t Dim> using ndarray_opaque_type = ndarray_opaque<Dim, opaque_format_multi_array>;
template<std::size_t Dim> using ndarray_view_opaque_type = ndarray_view_opaque<Dim, opaque_format_multi_array>;

opaque_format_multi_array opaque_frame_format();
ndarray_opaque_type<0> make_opaque_frame(int i);
int opaque_frame_index(const ndarray_view_opaque_type<0>&, bool verify = false);
bool compare_opaque_frames(const ndarray_view_opaque_type<1>& frames, const std::vector<int>& is);


}}

#endif
