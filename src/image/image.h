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
	using masked_pixel_type = masked_elem<Pixel>;

	using view_type = ndarray_view<2, masked_pixel_type>;

	using cv_mat_type = cv::Mat_<Pixel>;
	using cv_mask_mat_type = cv::Mat_<bool>;

private:
	view_type view_;
	cv_mat_type mat_;
	cv_mask_mat_type mask_mat_;

public:
	image(const view_type& vw);
	
	const view_type& view() noexcept { return view_; }
	const cv_mat_type& cv_mat() { return mat_; }
	const cv_mask_mat_type& cv_mask_mat() { return mask_mat_; }
	
	void write_cv_mat_background(Pixel background);
	void read_cv_mat_background(Pixel background);
	
	void update_cv_mat();
	void commit_cv_mat();
};


template<typename Pixel>
image<Pixel> to_image(const ndarray_view<2, masked_elem<Pixel>>& vw) {
	return image<Pixel>(vw);
}

// TODO (shared) ownership of resources (e.g. image retains view_)
// TODO masked/unmasked
/*
template<typename Pixel_in, typename Pixel_out, typename Function>
void apply_kernel(image<Pixel_in>& in_img, image<Pixel_out>& out_img, const ndsize<2>& kernel_shp, Function&& func);
*/
}

#include "image.tcc"

#endif
