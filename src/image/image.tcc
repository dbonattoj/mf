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

#include "../ndarray/ndarray_view_cast.h"
#include <stdexcept>
#include <utility>

namespace mf {


template<typename Pixel>
image<Pixel>::image(const shape_type& shape) :
	mat_(shape[0], shape[1]) { }


template<typename Pixel>
image<Pixel>::image(const const_view_type& vw) {
	copy_to_opencv(vw, mat_);
}


template<typename Pixel>
image<Pixel>::image(const image& img) :
	image(img.view()) { }


template<typename Pixel>
image<Pixel>::image(const image&& img) :
	mat_(img.mat_) { }


template<typename Pixel>
auto image<Pixel>::operator=(const image& img) -> image& {
	img.mat_.copyTo(mat_);
	return *this;
}


template<typename Pixel>
auto image<Pixel>::operator=(image&& img) -> image& {
	mat_ = img.mat_;
	return *this;
}


template<typename Pixel>
auto image<Pixel>::view() -> view_type {
	return to_ndarray_view(mat_);
}


template<typename Pixel>
auto image<Pixel>::view() const -> const_view_type {
	return to_ndarray_view(mat_);	
}



template<typename Pixel>
masked_image<Pixel>::masked_image(const shape_type& shape) :
	base(shape),
	mask_mat_(shape[0], shape[1]) { }


template<typename Pixel>
masked_image<Pixel>::masked_image(const masked_const_view_type& vw) {
	read(vw);
}


template<typename Pixel>
masked_image<Pixel>::masked_image(const masked_image& img) :
base(img) {
	img.mask_mat_.copyTo(mask_mat_);
}
	


template<typename Pixel>
masked_image<Pixel>::masked_image(const base& img) :
base(img) {
	mask_mat_.setTo(255);
}


template<typename Pixel>
masked_image<Pixel>::masked_image(masked_image&& img) : 
base(std::move(img)) {
	mask_mat_ = img.mask_mat_;
}


template<typename Pixel>
masked_image<Pixel>::masked_image(base&& img) :
base(std::move(img)) {
	mask_mat_ = img.mask_mat_;
}


template<typename Pixel>
auto masked_image<Pixel>::operator=(const masked_image& img) -> masked_image& {
	base::operator=(img);
	img.mask_mat_.copyTo(mask_mat_);
	return *this;
}


template<typename Pixel>
auto masked_image<Pixel>::operator=(const base& img) -> masked_image& {
	base::operator=(img);
	mask_mat_.setTo(255);
	return *this;
}


template<typename Pixel>
auto masked_image<Pixel>::operator=(masked_image&& img) -> masked_image& {
	base::operator=(std::move(img));
	mask_mat_ = img.mask_mat_;
	return *this;
}


template<typename Pixel>
auto masked_image<Pixel>::operator=(base&& img) -> masked_image& {
	base::operator=(std::move(img));
	mask_mat_.setTo(255);
	return *this;
}


template<typename Pixel>
void masked_image<Pixel>::read(const masked_const_view_type& vw) {
	copy_masked_to_opencv(vw, base::mat_, mask_mat_);
}


template<typename Pixel>
void masked_image<Pixel>::write(const masked_view_type& vw) const {
	copy_masked_to_ndarray_view(base::mat_, mask_mat_, vw);
}


}
