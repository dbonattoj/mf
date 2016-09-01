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


}
