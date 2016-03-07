#ifndef MF_OPENCV_NDARRAY_H_
#define MF_OPENCV_NDARRAY_H_

#include "opencv.h"
#include "color.h"
#include "ndarray_view.h"

namespace mf {

template<typename Array>
auto to_opencv_mat(const Array& arr) {
	// TODO not copy, need proper OpenCV support (constructor)
	static_assert(Array::dimension == 2, "ndarray dimension must be 2");
	using value_type = std::remove_const_t<typename Array::value_type>;
	std::size_t rows = arr.shape()[0], cols = arr.shape()[1];
	cv::Mat_<cv::Vec3b> mat(rows, cols);
	for(std::ptrdiff_t y = 0; y < rows; ++y) for(std::ptrdiff_t x = 0; x < cols; ++x) {
		rgb_color col = arr[y][x];
		mat(y, x) = cv::Vec3b(col.r, col.g, col.b);
	}
	return mat;
};

}

#endif
