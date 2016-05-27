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
#include <algorithm>
#include "masked_elem.h"

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


/// Copy the data in \a vw into the OpenCV Mat \a mat.
/** \a mat is created and allocated as needed using `cv::Mat::create()`, and \a mat owns the new copy of the data.
 ** `Elem` can not be a `masked_elem` type. */
template<std::size_t Dim, typename Elem>
void copy_to_opencv(const ndarray_view<Dim, const Elem>& vw, cv::Mat_<Elem>& mat) {
	int sizes[Dim];
	for(std::ptrdiff_t i = 0; i < Dim; ++i) sizes[i] = vw.shape()[i];

	mat.create(Dim, sizes);

	/// TODO check order of iterations
	std::transform(
		vw.begin(),
		vw.end(),
		mat.begin(),
		[](const Elem& elem) -> Elem { return elem; }
	);
}


/// Copy the data in the masked view \a vw into the OpenCV Mat \a mat, and the mask into \a mask_mat.
/** Like with \ref copy_to_opencv, \a mat is created and allocated as needed. \a vw is an \ref ndarray_view of
 ** \ref masked_elem elements.
 ** Non-null elements from `vw` are copied into \a mat, and \a mask_set gets written into \a mask_mat.
 ** Null elements from `vw` are not copied into \a mat, and 0 gets written into \a mask_mat. */
template<std::size_t Dim, typename Elem>
void copy_masked_to_opencv
(const ndarray_view<Dim, const masked_elem<Elem>>& vw, cv::Mat_<Elem>& mat, cv::Mat_<uchar>& mask_mat, uchar mask_set = 255) {
	int sizes[Dim];
	for(std::ptrdiff_t i = 0; i < Dim; ++i) sizes[i] = vw.shape()[i];

	mat.create(Dim, sizes);
	mask_mat.create(Dim, sizes);

	auto vw_it = vw.begin();
	auto mat_it = mat.begin();
	auto mask_mat_it = mask_mat.begin();
	
	for(; vw_it != vw.end(); ++vw_it, ++mat_it, ++mask_mat_it) {
		const masked_elem<Elem>& elem = *vw_it;
		if(is_null(elem)) {
			*mask_mat_it = 0;
		} else {
			*mask_mat_it = mask_set;
			*mat_it = elem;
		}
	}
}


/// Create `cv::Mat` header pointing at the same data as \a vw.
/** \a vw must have default strides without padding, and `Elem` cannot be a `masked_elem` type. */
template<std::size_t Dim, typename Elem>
cv::Mat_<Elem> to_opencv(const ndarray_view<Dim, Elem>& vw) {	
	if(vw.default_strides_padding() != 0)
		throw std::invalid_argument("ndarray_view must have default strides without padding to be converted into Mat");
	
	using opencv_type = cv::DataType<Elem>;
	using channel_type = typename opencv_type::channel_type;

	int sizes[Dim];
	for(std::ptrdiff_t i = 0; i < Dim; ++i) sizes[i] = vw.shape()[i];

	cv::Mat mat(
		Dim,
		sizes,
		opencv_type::type,
		reinterpret_cast<void*>(vw.start())
	);

	return cv::Mat_<Elem>(mat);
}


/// Create `ndarray_view` pointing at same data as OpenCV matrix \a mat.
/** Dimension must be specified if different from 2, because the OpenCV \a mat knows its dimension only runtime. */
template<std::size_t Dim = 2, typename Elem>
ndarray_view<Dim, Elem> to_ndarray_view(const cv::Mat_<Elem>& mat) {
	if(mat.dims != Dim) throw std::invalid_argument("OpenCV matrix in to_ndarray_view has incorrect dimension");
	Elem* start = reinterpret_cast<Elem*>(mat.data);
	ndcoord<Dim, int> shape(mat.size.p, mat.size.p + Dim);
	ndcoord<Dim, std::size_t> strides(mat.step.p, mat.step.p + Dim);
	return ndarray_view<Dim, Elem>(start, shape, strides);
}



/// Copy OpenCV Mat \a mat into the memory pointed to by \a vw.
/** \a vw and the memory if points to must have been created beforehand, and \a vw must have the same shape as \a mat.
 ** `Elem` cannot be a `masked_elem` type. */
template<std::size_t Dim, typename Elem>
void copy_to_ndarray_view(const cv::Mat_<Elem>& mat, const ndarray_view<Dim, Elem>& vw) {
	ndarray_view<Dim, Elem> tmp_vw = to_ndarray_view<Dim>(mat);
	if(tmp_vw.shape() != vw.shape()) throw std::invalid_argument("vw in copy_to_ndarray_view has incorrect shape");
	vw.assign(tmp_vw);
}


/// Copy OpenCV Mat \a mat with mask \a mask_mat into the memory pointed to by \a vw.
template<std::size_t Dim, typename Elem>
void copy_masked_to_ndarray_view
(const cv::Mat_<Elem>& mat, const cv::Mat_<uchar>& mask_mat, const ndarray_view<Dim, masked_elem<Elem>>& vw) {
	ndarray_view<Dim, Elem> tmp_vw = to_ndarray_view<Dim>(mat);
	ndarray_view<Dim, uchar> tmp_mask_vw = to_ndarray_view<Dim>(mask_mat);
	
	if(tmp_vw.shape() != vw.shape())
		throw std::invalid_argument("vw in copy_masked_to_ndarray_view has incorrect shape");
	if(tmp_vw.shape() != tmp_mask_vw.shape())
		throw std::invalid_argument("tmp_mask_vw in copy_masked_to_ndarray_view has incorrect shape");
	
	auto tmp_vw_it = tmp_vw.begin();
	auto tmp_mask_vw_it = tmp_mask_vw.begin();
	auto vw_it = vw.begin();
	for(; vw_it != vw.end(); ++tmp_vw_it, ++tmp_mask_vw_it, ++vw_it) {
		mask_type mask = *tmp_mask_vw_it;
		vw_it->mask = mask;
		if(mask) vw_it->elem = *tmp_vw_it;
	}
}



}

#endif
