#include <algorithm>

namespace mf { namespace node {

template<typename Color, typename Depth>
void warp<Color, Depth>::setup_() {
	output.define_frame_shape(image_input.frame_shape());
}


template<typename Color, typename Depth>
void warp<Color, Depth>::process_() {
}


}}
