#ifndef MF_IMAGE_VIEW_H_
#define MF_IMAGE_VIEW_H_

#include "../nd/ndarray_view.h"
#include "../opencv.h"
#include "../masked_elem.h"
#include "../nd/ndarray_view_cast.h"
#include <type_traits>

namespace mf {

template<typename Pixel>
class image_view {
public:
	using shape_type = ndsize<2>;
	using pixel_type = Pixel;
	
	using ndarray_view_type = ndarray_view<2, pixel_type>;
	
	using cv_mat_type = cv::Mat_<std::remove_const_t<pixel_type>>;
	using cv_mat_qualified_type = std::conditional_t<
		std::is_const<pixel_type>::value, const cv_mat_type, cv_mat_type
	>;

protected:
	using const_view = image_view<std::add_const_t<pixel_type>>;
	mutable cv_mat_type mat_;

public:
	image_view();
	explicit image_view(const ndarray_view_type&);
	explicit image_view(cv_mat_qualified_type&);
	image_view(const image_view&);
	virtual ~image_view() = default;
	
	image_view& operator=(const image_view&) = delete;

	void reset(const image_view&);
	operator const_view () const { return const_view(mat_); }

	bool is_null() const { return mat_.empty(); }
	explicit operator bool () const { return ! is_null(); }
			
	shape_type shape() const;
	
	ndarray_view_type array_view() const;
	cv_mat_qualified_type& cv_mat() const { return mat_; }
};



template<typename Pixel, typename Mask = byte>
class masked_image_view : public image_view<Pixel> {
	using base = image_view<Pixel>;

public:
	using typename base::shape_type;
	using typename base::pixel_type;
	using typename base::ndarray_view_type;
	using typename base::cv_mat_type;
	using typename base::cv_mat_qualified_type;
	
	using mask_type = Mask;
	using mask_ndarray_view_type = ndarray_view<2, mask_type>;

	using cv_mask_mat_type = cv::Mat_<std::remove_const_t<mask_type>>;
	using cv_mask_mat_qualified_type = std::conditional_t<
		std::is_const<mask_type>::value, const cv_mask_mat_type, cv_mask_mat_type
	>;

private:
	using typename base::const_view;
	using const_masked_view = masked_image_view<std::add_const_t<pixel_type>, std::add_const_t<mask_type>>;

	mutable cv_mask_mat_type mask_mat_;

public:
	masked_image_view();
	masked_image_view(const masked_image_view&);
	masked_image_view(masked_image_view&&);
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

#include "image_view.tcc"

#endif
