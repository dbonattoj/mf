#include "ndarray_format.h"

namespace mf {

bool operator==(const ndarray_format& a, const ndarray_format& b) {
	return
		(a.elem_size() == b.elem_size()) &&
		(a.elem_alignment() == b.elem_alignment()) &&
		(a.length() == b.length()) &&
		(a.stride() == b.stride());
}


bool operator!=(const ndarray_format& a, const ndarray_format& b) {
	return
		(a.elem_size() != b.elem_size()) ||
		(a.elem_alignment() != b.elem_alignment()) ||
		(a.length() != b.length()) ||
		(a.stride() != b.stride());
}

}
