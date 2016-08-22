#ifndef MF_MASKED_IMAGE_VIEW_H_
#define MF_MASKED_IMAGE_VIEW_H_

#include "image_view.h"

namespace mf {

/// View to two-dimensional image with mask.
/** Image and mask represented as two separate OpenCV Mat.
 ** \a Pixel may be const or non-const. \a Mask must be non-const, but the actually used `mask_type` will get the
 ** same constness as `pixel_type`. */
template<typename Pixel, typename Mask = byte>
class masked_image_view : public image_view<Pixel> {
	static_assert(! std::is_const<Mask>::value, "masked_image_view Mask must not be const");
	using base = image_view<Pixel>;

public:
	using typename base::shape_type;
	using typename base::pixel_type;
	using typename base::ndarray_view_type;
	using typename base::cv_mat_type;
	using typename base::cv_mat_qualified_type;
	
	using mask_type = std::conditional_t<std::is_const<Pixel>::value, const Mask, Mask>;
	using mask_ndarray_view_type = ndarray_view<2, mask_type>;

	using cv_mask_mat_type = cv::Mat_<std::remove_const_t<mask_type>>;
	using cv_mask_mat_qualified_type = std::conditional_t<
		std::is_const<mask_type>::value, const cv_mask_mat_type, cv_mask_mat_type
	>;

private:
	using typename base::const_view;
	using const_masked_view = masked_image_view<std::add_const_t<pixel_type>, Mask>;

	mutable cv_mask_mat_type mask_mat_;

public:
	masked_image_view();
	masked_image_view(const masked_image_view&);
	masked_image_view(const ndarray_view_type&, const mask_ndarray_view_type&);
	masked_image_view(cv_mat_qualified_type&, cv_mask_mat_qualified_type&);

	masked_image_view& operator=(const masked_image_view&) = delete;

	void reset(const masked_image_view&);
	operator const_masked_view () const { return const_masked_view(base::mat_, mask_mat_); }
	operator const_view () const { return const_view(base::mat_); }

	mask_ndarray_view_type mask_array_view() const;
	cv_mask_mat_qualified_type& cv_mask_mat() const { return mask_mat_; }
};


template<typename Pixel>
auto to_masked_image_view(const ndarray_view<2, masked_elem<Pixel>>& masked_vw) {
	using image_type = masked_image_view<Pixel, byte>;
	auto vw = ndarray_view_cast<typename image_type::view_type>(masked_vw);
	auto mask_vw = ndarray_view_cast<typename image_type::mask_view_type>(masked_vw);
	return image_type(vw, mask_vw);
}


}

#include "masked_image_view.tcc"

#endif
