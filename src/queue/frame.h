#ifndef MF_FRAME_H_
#define MF_FRAME_H_

#include "../ndarray/generic/ndarray_view_generic.h"
#include "../ndarray/generic/ndarray_timed_view_generic.h"
#include "../ndarray/generic/ndarray_generic.h"

namespace mf {

using timed_frame_array_view = ndarray_timed_view_generic<1>;
using frame_array_view = ndarray_view_generic<1>;
using frame_view = ndarray_view_generic<0>;
using frame_array_properties = ndarray_generic_properties;

}

#endif
