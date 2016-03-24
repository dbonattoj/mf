#ifndef MF_OPENCV_NDARRAY_H_
#define MF_OPENCV_NDARRAY_H_

#include "opencv.h"
#include "color.h"

namespace mf {

template<typename Array>
auto to_opencv_mat(const Array& arr) {
	using value_type = typename Array::value_type;

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
		cv::DataType<value_type>::type,
		reinterpret_cast<void*>(arr.start()),
		steps
	);
	cv::Mat_<value_type> mat_(mat);
	
	return mat_;	
};

}

#endif
