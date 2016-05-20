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
/** Represented using an `ndarray_view`, and the corresponding OpenCV `Mat_` pointing to same data. */
template<typename Pixel>
class image {
public:
	using pixel_type = Pixel;
	using view_type = ndarray_view<2, pixel_type>;
	using cv_mat_type = cv::Mat_<unmasked_type<pixel_type>>;

private:
	view_type view_;
	cv_mat_type mat_;

public:
	image(const view_type& vw);
	
	const view_type& view() noexcept { return view_; }
	const cv_mat_type& cv_mat() { return mat_; }
	
	virtual void update_cv_mat();
	virtual void commit_cv_mat();
};


/// Two-dimensional masked image with given pixel type.
/** Derived from \ref image with `masked_elem<Pixel>` pixel, with additional OpenCV `Mat_` for binary mask. */
template<typename Pixel>
class masked_image : public image<masked_elem<Pixel>> {
	using base = image<masked_elem<Pixel>>;
	
public:
	using pixel_type = typename base::pixel_type;
	using masked_pixel_type = masked_elem<Pixel>;
	using view_type = typename base::view_type;
	using cv_mat_type = typename base::cv_mat_type;
	using cv_mask_mat_type = cv::Mat_<bool>;	

private:
	cv_mask_mat_type mask_mat_;

public:
	masked_image(const view_type& vw);

	const cv_mask_mat_type& cv_mask_mat() { return mask_mat_; }

	void update_cv_mat();
	void commit_cv_mat();
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


}

#include "image.tcc"

#endif
