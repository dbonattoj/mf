#include "opaque_format.h"

namespace mf {

auto opaque_format::part_array_view(std::ptrdiff_t index, frame_ptr frm) const -> part_array_view_type {
	extracted_part prt = extract_part(index);
	
	Assert(ptr.format->has_array_format());
	ndarray_format arr_frm = ptr.format->array_format();
	
	frame_ptr start = advance_raw_ptr(frame_ptr, prt.offset);
	auto shape = make_ndsize(arr_frm.length());
	auto strides = make_ndptrdiff(arr_frm.stride());
	
	return part_array_view_type(start, shape, strides, frm);
}


auto opaque_format::part_array_view(std::ptrdiff_t index, const_frame_ptr frm) const -> part_const_array_view_type {
	extracted_part prt = extract_part(index);
	
	Assert(ptr.format->has_array_format());
	ndarray_format arr_frm = ptr.format->array_format();
	
	frame_ptr start = advance_raw_ptr(frame_ptr, prt.offset);
	auto shape = make_ndsize(arr_frm.length());
	auto strides = make_ndptrdiff(arr_frm.stride());
	
	return part_const_array_view_type(start, shape, strides, frm);
}


}
