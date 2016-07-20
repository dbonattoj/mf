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

#ifndef MF_IMAGE_H_
#define MF_IMAGE_H_

#include "../ndarray/ndarray_view.h"
#include "../opencv.h"
#include "../masked_elem.h"
#include <type_traits>

namespace mf {

/// Two-dimensional image with given pixel type.
template<typename Pixel>
class image {
public:
	using shape_type = ndsize<2>;

	using pixel_type = Pixel;
	using view_type = ndarray_view<2, pixel_type>;
	using const_view_type = ndarray_view<2, const pixel_type>;
	
	using cv_mat_type = cv::Mat_<pixel_type>;

protected:
	cv_mat_type mat_; ///< OpenCV Mat, owns the data.

	image() = default;

public:
	explicit image(const shape_type&);
	explicit image(const const_view_type&);
	explicit image(const cv_mat_type&);
	
	image(const image&);
	image(const image&&);
	
	image& operator=(const image&);
	image& operator=(image&&);
	
	shape_type shape() const;
	
	view_type view();
	const_view_type view() const;
	
	cv_mat_type& cv_mat() noexcept { return mat_; }
	const cv_mat_type& cv_mat() const noexcept { return mat_; }
};


/// Two-dimensional masked image with given pixel type.
template<typename Pixel>
class masked_image : public image<Pixel> {
	using base = image<Pixel>;

public:
	using typename base::shape_type;
	using typename base::pixel_type;
	using typename base::view_type;
	using typename base::const_view_type;
	using typename base::cv_mat_type;
	
	using masked_pixel_type = masked_elem<Pixel>;
	
	using mask_view_type = ndarray_view<2, pixel_type>;
	using masked_view_type = ndarray_view<2, masked_pixel_type>;
	using masked_const_view_type = ndarray_view<2, const masked_pixel_type>;
	
	using cv_mask_mat_type = cv::Mat_<uchar>;
	
private:
	cv_mask_mat_type mask_mat_; ///< OpenCV mask Mat, owns binary mask of data.

public:
	explicit masked_image(const shape_type&);
	explicit masked_image(const const_view_type&);
	explicit masked_image(const masked_const_view_type&);	
	
	masked_image(const masked_image&);
	masked_image(const base&);
	masked_image(masked_image&&);
	masked_image(base&&);

	masked_image& operator=(const masked_image&);
	masked_image& operator=(const base&);
	masked_image& operator=(masked_image&&);
	masked_image& operator=(base&&);

	cv_mask_mat_type& cv_mask_mat() noexcept { return mask_mat_; }
	const cv_mask_mat_type& cv_mask_mat() const noexcept { return mask_mat_; }
	
	void read_masked(const masked_const_view_type&);
	void write_masked(const masked_view_type&) const;
};



/// Create \ref image for given \ref ndarray_view.
template<typename Pixel>
image<Pixel> to_image(const ndarray_view<2, Pixel>& vw) {
	return image<Pixel>(vw);
}

template<typename Pixel>
masked_image<Pixel> to_image(const ndarray_view<2, masked_elem<Pixel>>& vw) {
	return masked_image<Pixel>(vw);
}


template<typename Pixel>
image<Pixel> to_image(const cv::Mat_<Pixel>& mat) {
	return image<Pixel>(mat);
}



}

#include "image.tcc"

#endif
