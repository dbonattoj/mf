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
