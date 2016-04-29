#ifndef MF_OPENCV_H_
#define MF_OPENCV_H_

#include <opencv2/opencv.hpp>

#include "color.h"

#include <iostream>

namespace cv { // in OpenCV namespace
	template<>
	class DataType<::mf::rgb_color> {
	public:
		using value_type = mf::rgb_color;
		using work_type = int;
		using channel_type = uchar;
		enum {
			generic_type = 0,
			depth = DataDepth<channel_type>::value,
			channels = 3,
			fmt = ((channels - 1)<<8) + DataDepth<channel_type>::fmt,
			type = CV_MAKETYPE(depth, channels)
		};
		using vec_type = Vec<channel_type, channels>;
	};
	
	template<>
	class DataType<::mf::ycbcr_color> {
	public:
		using value_type = mf::ycbcr_color;
		using work_type = int;
		using channel_type = uchar;
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

namespace mf {

template<typename Array>
auto to_opencv_mat(const Array& arr) {
	using value_type = typename Array::value_type;
	using opencv_type = cv::DataType<value_type>;
	using channel_type = typename opencv_type::channel_type;

	int sizes[Array::dimension];
	std::size_t steps[Array::dimension];
	
	for(std::ptrdiff_t i = 0; i < Array::dimension; ++i) {
		sizes[i] = arr.shape()[i];
		steps[i] = arr.strides()[i];
	}
	
	// need intermadiary Mat, because Mat_ constructor not properly implemented in OpenCV 2.4
	// no data is copied
	cv::Mat mat(
		Array::dimension,
		sizes,
		opencv_type::type,
		reinterpret_cast<void*>(arr.start()),
		steps
	);
	cv::Mat_<value_type> mat_(mat);
	
	return mat_;	
};

}

#endif
