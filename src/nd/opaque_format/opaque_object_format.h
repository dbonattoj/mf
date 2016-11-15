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

#ifndef MF_NDARRAY_OPAQUE_OBJECT_FORMAT_H_
#define MF_NDARRAY_OPAQUE_OBJECT_FORMAT_H_

#include "opaque_format.h"
#include <type_traits>

namespace mf {

class opaque_object_format_base : public opaque_format {
public:
	using object_base_ptr = void*;
	using object_base_const_ptr = const void*;
	
	static object_base_ptr obj_ptr(frame_ptr);
	static object_base_const_ptr obj_ptr(const_frame_ptr);
	
	opaque_object_format_base(std::size_t size, std::size_t alignment_requirement, bool is_pod);
};

/// Format of \ref ndarray_view_opaque frame containing object instance of type \a Object.
/** \a Object can be non-POD type, and it is constructed and destructed with frame.
 ** \a Object must have public default constructor, destructor, copy assignment operator and comparison operator. */
template<typename Object>
class opaque_object_format : public opaque_object_format_base {
public:	
	using object_type = Object;

	static object_type& obj(frame_ptr);
	static const object_type& obj(const_frame_ptr);

	opaque_object_format();
	
	bool compare(const opaque_format&) const override;

	void copy_frame(frame_ptr destination, const_frame_ptr origin) const override;
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const override;
	void construct_frame(frame_ptr) const override;
	void destruct_frame(frame_ptr) const override;
	void initialize_frame(frame_ptr) const override;
};

}

#include "opaque_object_format.tcc"

#endif
