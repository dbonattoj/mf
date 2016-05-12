#include "../ndarray/ndarray_view_cast.h"
#include <stdexcept>

namespace mf {

/*
template<typename Pixel_in, typename Pixel_out, typename Function>
void apply_kernel(image<Pixel_in>& in_img, image<Pixel_out>& out_img, const ndsize<2>& kernel_shp, Function&& func) {
	MF_EXPECTS(kernel_shp[0] % 2 != 0);
	MF_EXPECTS(kernel_shp[1] % 2 != 0);
	
	if(in_img.view().shape() != out_img.view().shape())
		throw std::invalid_argument("input and output of apply_kernel must have same shapes");

	ndptrdiff<2> kernel_shp_half = kernel_shp / 2; // rounded down
	
	for(auto out_it = out_img.view().begin(); out_it != out_img.view().end(); ++out_it) {
		ndptrdiff<2> coord = out_it.coordinates();
		ndspan<2> span(coord - kernel_shp_half, coord + kernel_shp_half + ndptrdiff<2>(1));
		MF_ASSERT(span.shape() == kernel_shp);
		
		ndspan<2> full_span(0, in_img.view().shape());
		ndspan<2> truncated_span = span_intersection(full_span, span);
		
		auto in_section = in_img.view().section(truncated_span);
				
		auto center = make_ndptrdiff(
			kernel_shp_half[0] + std::min(span.start_pos()[0], std::ptrdiff_t(0)),
			kernel_shp_half[1] + std::min(span.start_pos()[1], std::ptrdiff_t(0))
		);

		MF_ASSERT(span.includes(center));
		MF_ASSERT(&(*out_it) == &in_section.at(center));
		
		*out_it = func(in_section, center);
	}
}
*/


template<typename Pixel>
image<Pixel>::image(const view_type& vw) :
	view_(vw),
	mat_( to_opencv_mat( ndarray_view_cast< ndarray_view<2, Pixel> >(vw) ) ),
	mask_mat_( to_opencv_mat( ndarray_view_cast< ndarray_view<2, bool> >(vw) ) ) { }
	

template<typename Pixel>
void image<Pixel>::write_cv_mat_background(Pixel background) {
	cv::Mat_<Pixel> dest_mat;
	dest_mat.setTo(background);
	mat_.copyTo(dest_mat, mask_mat_);
	dest_mat.copyTo(mat_);
}


template<typename Pixel>
void image<Pixel>::read_cv_mat_background(Pixel background) {
	for(std::ptrdiff_t y = 0; y < view_.shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < view_.shape()[1]; ++x) {
		mask_mat_(y, x) = (mat_(y, x) != background);
	}
}
	
template<typename Pixel>
void image<Pixel>::update_cv_mat() {
	to_opencv_mat( ndarray_view_cast< ndarray_view<2, Pixel> >(view_) ).copyTo(mat_);
	to_opencv_mat( ndarray_view_cast< ndarray_view<2, bool> >(view_) ).copyTo(mask_mat_);
}
	
template<typename Pixel>
void image<Pixel>::commit_cv_mat() {
	for(std::ptrdiff_t y = 0; y < view_.shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < view_.shape()[1]; ++x) {
		if( mask_mat_(y, x) ) view_[y][x] = mat_(y, x);
		else view_[y][x] = masked_pixel_type::null();
	}
}


}
