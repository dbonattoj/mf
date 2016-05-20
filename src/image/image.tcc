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

namespace mf {

template<typename Pixel>
image<Pixel>::image(const view_type& vw) :
	view_(vw),
	mat_( to_opencv_mat( ndarray_view_cast< ndarray_view<2, Pixel> >(vw) ) ) { }
	// casts the element mask in the ndarray, if any
	
template<typename Pixel>
void image<Pixel>::update_cv_mat() {
	to_opencv_mat( ndarray_view_cast< ndarray_view<2, Pixel> >(view_) ).copyTo(mat_);
}
	
template<typename Pixel>
void image<Pixel>::commit_cv_mat() {
	for(std::ptrdiff_t y = 0; y < view_.shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < view_.shape()[1]; ++x) {
		view_[y][x] = mat_(y, x);
	}
}




template<typename Pixel>
masked_image<Pixel>::masked_image(const view_type& vw) :
	base(vw),
	mask_mat_(vw.shape()[0], vw.shape()[1]) { }
	
template<typename Pixel>
void masked_image<Pixel>::update_cv_mat() {
	base::update_cv_mat();
	
	for(std::ptrdiff_t y = 0; y < base::view().shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < base::view().shape()[1]; ++x) {
		mask_mat_(y, x) = ! base::view()[y][x].is_null();
	}
}
	
template<typename Pixel>
void masked_image<Pixel>::commit_cv_mat() {
	base::commit_cv_mat();
	
	for(std::ptrdiff_t y = 0; y < base::view().shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < base::view().shape()[1]; ++x) {
		if(mask_mat_(y, x)) base::view()[y][x] = masked_pixel_type::null();
	}
}



}
