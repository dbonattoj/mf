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
	auto planar_rgb() {
		raw_video_frame_format<rgb_color> format;
		return format;
	}
	
	auto interleaved_rgb() {
		raw_video_frame_format<rgb_color> format;
		format.interleaved = true;
		return format;
	}
	
	auto interleaved_ycbcr() {
		raw_video_frame_format<ycbcr_color> format;
		format.interleaved = true;
		return format;
	}
	
	auto planar_ycbcr_4_4_4() {
		raw_video_frame_format<ycbcr_color> format;
		return format;
	}
	
	auto planar_ycbcr_4_2_0() {
		raw_video_frame_format<ycbcr_color> format;
		format.component_scale_x[1] = format.component_scale_x[2] = 2;
		format.component_scale_y[1] = format.component_scale_y[2] = 2;
		return format;
	}
}

}

#endif
