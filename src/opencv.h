#ifndef MF_OPENCV_H_
#define MF_OPENCV_H_

#include <opencv2/opencv.hpp>
#include <cstdint>

#include "color.h"

namespace cv { // in OpenCV namespace
	template<>
	class DataType<mf::rgb_color> {
	public:
		using value_type = mf::rgb_color;
		using work_type = int;
		using channel_type = std::uint8_t;
		enum {
			generic_type = 0,
			depth = DataDepth<channel_type>::fmt,
			channels = 3,
			fmt = ((channels - 1)<<8) + DataDepth<channel_type>::fmt,
			type = CV_MAKETYPE(depth, channels)
		};
		using vec_type = Vec<channel_type, channels>;
	};
	
	template<>
	class DataType<mf::ycbcr_color> {
	public:
		using value_type = mf::ycbcr_color;
		using work_type = int;
		using channel_type = std::uint8_t;
		enum {
			generic_type = 0,
			depth = DataDepth<channel_type>::fmt,
			channels = 3,
			fmt = ((channels - 1)<<8) + DataDepth<channel_type>::fmt,
			type = CV_MAKETYPE(depth, channels)
		};
		using vec_type = Vec<channel_type, channels>;
	};
}

#endif
