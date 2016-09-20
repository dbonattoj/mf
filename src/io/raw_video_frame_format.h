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

#ifndef MF_RAW_VIDEO_FRAME_FORMAT_H_
#define MF_RAW_VIDEO_FRAME_FORMAT_H_

#include "../color.h"
#include <array>

namespace mf {

template<typename Elem>
struct raw_video_frame_format {
	constexpr static std::size_t num_components = elem_traits<Elem>::components;
	using component_type = typename elem_traits<Elem>::scalar_type;

	bool interleaved = false;
	std::array<unsigned, num_components> component_scale_x;
	std::array<unsigned, num_components> component_scale_y;
	
	raw_video_frame_format() { component_scale_x.fill(1); component_scale_y.fill(1); }
};

namespace raw_video_frame_formats {
	raw_video_frame_format<rgb_color> planar_rgb();
	raw_video_frame_format<rgb_color> interleaved_rgb();
	raw_video_frame_format<ycbcr_color> interleaved_ycbcr();
	raw_video_frame_format<ycbcr_color> planar_ycbcr_4_4_4();
	raw_video_frame_format<ycbcr_color> planar_ycbcr_4_2_0();
}

}

#endif
