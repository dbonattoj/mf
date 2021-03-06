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

#ifndef MF_FRAME_EXPORTER_H_
#define MF_FRAME_EXPORTER_H_

#include "../common.h"
#include "../nd/ndcoord.h"
#include "../nd/ndarray_view.h"

namespace mf {

/// Frame exporter, abstract base class.
template<std::size_t Dim, typename Elem>
class frame_exporter {
public:
	using frame_view_type = ndarray_view<Dim, const Elem>;
	using frame_shape_type = ndsize<Dim>;
	
	using elem_type = Elem;
	constexpr static std::size_t dimension = Dim;

	virtual ~frame_exporter() { }
		
	virtual void setup(const frame_shape_type&) { }
	virtual void write_frame(const frame_view_type&) = 0;
	virtual void close() { }
};

}

#endif
