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


bool opaque_ndarray_format::compare(const opaque_format& frm) {
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
