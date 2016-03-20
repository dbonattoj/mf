#ifndef MF_OPENCV_NDARRAY_H_
#define MF_OPENCV_NDARRAY_H_

#include "opencv.h"
#include "color.h"
#include "ndarray/ndarray_view.h"

namespace mf {

template<std::size_t Dim>
auto to_opencv_mat(const ndarray_view<Dim, rgb_color>& vw) {
	std::vector<int> sizes(vw.shape().begin(), vw.shape().end());		
	std::vector<std::size_t> steps(vw.strides().begin(), vw.strides().end());
	//for(std::size_t& step : steps) step /= 4;
	//for(int& size : sizes) size *= 4;
	cv::Vec4b* data = reinterpret_cast<cv::Vec4b*>(vw.start());
	return cv::Mat_<cv::Vec4b>(Dim, sizes.data(), data, steps.data());
};

}

#endif
