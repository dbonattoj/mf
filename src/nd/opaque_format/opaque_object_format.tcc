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

#include <iostream>
namespace mf {

template<typename Object>
inline auto opaque_object_format<Object>::obj(frame_ptr frame) -> object_type& {
	return *reinterpret_cast<object_type*>(frame);
}


template<typename Object>
inline auto opaque_object_format<Object>::obj(const_frame_ptr frame) -> const object_type& {
	return *reinterpret_cast<const object_type*>(frame);
}


template<typename Object>
opaque_object_format<Object>::opaque_object_format() {
	set_frame_size_(sizeof(Object));
	set_frame_alignment_requirement_(alignof(Object));
	set_pod_(std::is_pod<Object>::value);
	set_contiguous_(true);
}


template<typename Object>
bool opaque_object_format<Object>::compare(const opaque_format& frm) const {
	return (typeid(frm) == typeid(opaque_object_format));
}


template<typename Object>
void opaque_object_format<Object>::copy_frame(frame_ptr destination, const_frame_ptr origin) const {
	obj(destination) = obj(origin);
}


template<typename Object>
bool opaque_object_format<Object>::compare_frame(const_frame_ptr a, const_frame_ptr b) const {
	return (obj(a) == obj(b));
}


template<typename Object>
void opaque_object_format<Object>::construct_frame(frame_ptr frame) const {
	new(frame) Object();
}


template<typename Object>
void opaque_object_format<Object>::destruct_frame(frame_ptr frame) const {
	obj(frame).~Object();
}


template<typename Object>
void opaque_object_format<Object>::initialize_frame(frame_ptr frame) const {
	obj(frame) = Object();
}


}
