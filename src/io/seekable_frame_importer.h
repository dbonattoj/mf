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

#ifndef MF_SEEKABLE_FRAME_IMPORTER_H_
#define MF_SEEKABLE_FRAME_IMPORTER_H_

#include "frame_importer.h"
#include "../common.h"
#include "../nd/ndcoord.h"
#include "../nd/ndarray_view.h"

namespace mf {


/// Seekable frame importer, abstract base class.
/** Can change position in file using seek(). Keeps track of absolute frame index (called current time). Total duration
 ** of file is known after construction. */
template<std::size_t Dim, typename Elem>
class seekable_frame_importer : public frame_importer<Dim, Elem> {
	using base = frame_importer<Dim, Elem>;
	
protected:
	seekable_frame_importer(const ndsize<Dim>& frame_shape) :
		base(frame_shape) { }

public:
	using typename base::frame_view_type;
	constexpr static bool is_seekable = true;
	
	virtual time_unit current_time() const = 0;
	virtual time_unit total_duration() const = 0;
	
	virtual void seek(time_unit) = 0;
	
	void read_frame_at(const frame_view_type& vw, time_unit t) {
		this->seek(t);
		this->read_frame(vw);
	}
};


}

#endif

