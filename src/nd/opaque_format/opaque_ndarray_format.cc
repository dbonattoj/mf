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

#include "opaque_ndarray_format.h"

namespace mf {

opaque_ndarray_format::opaque_ndarray_format(const ndarray_format& form) :
	array_format_(form)
{
	set_frame_size_(array_format_.frame_size());
	set_frame_alignment_requirement_(array_format_.frame_alignment_requirement());
	set_pod_(true);
	set_contiguous_(array_format_.is_contiguous());
}


bool opaque_ndarray_format::compare(const opaque_format& frm) const {
	if(typeid(opaque_ndarray_format) == typeid(frm))
		return (static_cast<const opaque_ndarray_format&>(frm) == *this);
	else
		return false;
}

	
void opaque_ndarray_format::copy_frame(frame_ptr destination, const_frame_ptr origin) const {
	ndarray_data_copy(destination, origin, array_format_);
}


bool opaque_ndarray_format::compare_frame(const_frame_ptr a, const_frame_ptr b) const {
	return ndarray_data_compare(a, b, array_format_);
}

}
