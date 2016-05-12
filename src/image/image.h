#ifndef MF_IMAGE_H_
#define MF_IMAGE_H_

#include "../ndarray/ndarray_view.h"
#include "../opencv.h"

namespace mf {

template<typename Pixel>
class image {
public:
	using view_type = ndarray_view<2, Pixel>;
	using const_view_type = ndarray_view<2, const Pixel>;
	using cv_mat_type = cv::Mat_<Pixel>;

private:
	view_type view_;
	cv_mat_type mat_;

public:
	image(const view_type& vw) :
		view_(vw), mat_(to_opencv_mat(vw)) { }
	
	const view_type& view() noexcept { return view_; }
	const_view_type view() const noexcept { return view_; }
	const_view_type cview() const noexcept { return view_; }
	
	const cv_mat_type& cv_mat() const { return mat_; }
	cv_mat_type& cv_mat() { return mat_; }
};


template<typename Pixel_in, typename Pixel_out, typename Function>
void apply_kernel(const image<Pixel_in>& in_img, image<Pixel_out>& out_img, const ndsize<2>& kernel_shp, Function func);

template<typename Pixel_in, typename Pixel_out, typename Kernel>
void convolve_kernel(const image<Pixel_in>& input_img, image<Pixel_out>& output_img, const Kernel& kernel);


}

#include "image.tcc"

#endif
