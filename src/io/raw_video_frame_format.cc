#include "raw_video_frame_format.h"

namespace mf {
	
namespace raw_video_frame_formats {
	raw_video_frame_format<rgb_color> planar_rgb() {
		raw_video_frame_format<rgb_color> format;
		return format;
	}
	
	raw_video_frame_format<rgb_color> interleaved_rgb() {
		raw_video_frame_format<rgb_color> format;
		format.interleaved = true;
		return format;
	}
	
	raw_video_frame_format<ycbcr_color> interleaved_ycbcr() {
		raw_video_frame_format<ycbcr_color> format;
		format.interleaved = true;
		return format;
	}
	
	raw_video_frame_format<ycbcr_color> planar_ycbcr_4_4_4() {
		raw_video_frame_format<ycbcr_color> format;
		return format;
	}
	
	raw_video_frame_format<ycbcr_color> planar_ycbcr_4_2_0() {
		raw_video_frame_format<ycbcr_color> format;
		format.component_scale_x[1] = format.component_scale_x[2] = 2;
		format.component_scale_y[1] = format.component_scale_y[2] = 2;
		return format;
	}
}

}
