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
image<Pixel>::image(const view_type& vw) : view_(vw) {
	update_cv_mat();
}
	
template<typename Pixel>
void image<Pixel>::update_cv_mat() {
	to_opencv_mat( ndarray_view_cast<unmasked_view_type>(view_) ).copyTo(mat_);
}
	
template<typename Pixel>
void image<Pixel>::commit_cv_mat() {
	for(auto coord : make_ndspan(view().shape()))
		view_.at(coord) = mat_(coord[0], coord[1]);
}




template<typename Pixel>
masked_image<Pixel>::masked_image(const view_type& vw) : base(vw) {
	to_opencv_mat(mask_view()).copyTo(mask_mat_);
}
	
template<typename Pixel>
void masked_image<Pixel>::update_cv_mat() {
	base::update_cv_mat();
	to_opencv_mat(mask_view()).copyTo(mask_mat_);
}
	
template<typename Pixel>
void masked_image<Pixel>::commit_cv_mat() {
	base::commit_cv_mat();

	for(auto coord : make_ndspan(base::view().shape()))
		if(! mask_mat_(coord[0], coord[1])) base::view().at(coord) = masked_pixel_type::null();
}



}
