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

#ifndef MF_OPENCV_H_
#define MF_OPENCV_H_

#include <opencv2/opencv.hpp>

#include "color.h"
#include "ndarray/ndcoord.h"
#include "ndarray/ndarray.h"

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


/*
/// Convert `ndarray_view` to OpenCV `Mat_` object, without copying data.
/** Dimension, shape and strides of the resulting `Mat_` are adjusted to match the `ndarray_view` \a arg.
 ** The element type of \a arr must have an OpenCV `DataType<>` specialization.
 ** As many OpenCV functions do not support `Mat_` with non-default strides, it may be necessary to copy the array
 ** to a new `Mat_` afterwards.  * /
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
*/

template<std::size_t Dim, typename Elem>
auto to_opencv_mat(const ndarray_view<Dim, Elem>& vw) {
	using opencv_type = cv::DataType<Elem>;
	using channel_type = typename opencv_type::channel_type;

	int sizes[Dim];
	for(std::ptrdiff_t i = 0; i < Dim; ++i) sizes[i] = vw.shape()[i];

/*	if(vw.has_default_strides()) {
		cv::Mat mat(
			Dim,
			sizes,
			opencv_type::type,
			reinterpret_cast<void*>(vw.start())
		);
		cv::Mat_<Elem> mat_(mat);
		return mat_;

	} else {*/
		ndarray<Dim, Elem> arr(vw);
		cv::Mat mat(
			Dim,
			sizes,
			opencv_type::type,
			reinterpret_cast<void*>(arr.start())
		);
		cv::Mat_<Elem> mat_(mat);
		cv::Mat_<Elem> mat_copy_;
		mat_.copyTo(mat_copy_);
		return mat_copy_;
		

}


template<std::size_t Dim, typename Elem>
bool copy_less_convertible_to_opencv_mat(const ndarray_view<Dim, Elem>& vw) {
	return vw.has_default_strides();
}



template<std::size_t Dim, typename T>
auto to_ndarray_view(const cv::Mat_<T>& mat) {
	T* start = reinterpret_cast<T*>(mat.data);
	ndcoord<Dim, int> shape(mat.size.p, mat.size.p + Dim);
	ndcoord<Dim, std::size_t> strides(mat.step.p, mat.step.p + Dim);
	return ndarray_view<Dim, T>(start, shape, strides);
}


}

#endif