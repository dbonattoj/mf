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

#ifndef MF_FRAME_IMPORTER_H_
#define MF_FRAME_IMPORTER_H_

#include "../common.h"
#include "../nd/ndcoord.h"
#include "../nd/ndarray_view.h"

namespace mf {

/// Frame importer, abstract base class.
/** Each frame is an ndarray<Dim, Elem> with fixed shape. Frames are read sequentially with read_frame(),
 ** and reached_end() becomes true when after last frame was read. */
template<std::size_t Dim, typename Elem>
class frame_importer {
private:
	ndsize<Dim> frame_shape_;

protected:
	frame_importer(const ndsize<Dim>& frame_shape) :
		frame_shape_(frame_shape) { }

public:
	using frame_view_type = ndarray_view<Dim, Elem>;
	using elem_type = Elem;
	constexpr static std::size_t dimension = Dim;
	constexpr static bool is_seekable = false;

	virtual ~frame_importer() { }
	
	const ndsize<Dim>& frame_shape() { return frame_shape_; }
	
	virtual void read_frame(const frame_view_type&) = 0;
	virtual bool reached_end() const = 0;
};


}

#endif

