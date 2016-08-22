#include <utility>

namespace mf {

template<typename Pixel>
image_view<Pixel>::image_view() { }


template<typename Pixel>
image_view<Pixel>::image_view(const ndarray_view_type& vw) :
	mat_(to_opencv(vw)) { }


template<typename Pixel>
image_view<Pixel>::image_view(cv_mat_qualified_type& mat) :
	mat_(mat) { }


template<typename Pixel>
image_view<Pixel>::image_view(const image_view<std::remove_const_t<pixel_type>>& im) :
	mat_(im.cv_mat()) { }
	

template<typename Pixel>
image_view<Pixel>::image_view(image_view&& im) :
	mat_(std::move(im.mat_)) { }


template<typename Pixel>
auto image_view<Pixel>::shape() const -> shape_type {
	return make_ndsize(mat_.size[0], mat_.size[1]);
}


template<typename Pixel>
auto image_view<Pixel>::array_view() const -> ndarray_view_type {
	if(is_null()) return ndarray_view_type::null();
	else return to_ndarray_view(mat_);
}


template<typename Pixel>
void image_view<Pixel>::reset(const image_view& im) {
	mat_ = im.mat_;
}
	

template<typename Pixel>
auto image_view<Pixel>::operator=(const image_view& im) -> image_view& {
	im.mat_.copyTo(mat_);
	return *this;
}


template<typename Pixel>
auto image_view<Pixel>::operator=(image_view&& im) -> image_view& {
	 mat_ = im.mat_;
	 return *this;
}



///////////////


template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view() { }


template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view(const ndarray_view_type& vw, const mask_ndarray_view_type& mask_vw) :
	base(vw),
	mask_mat_(to_opencv(mask_vw)) { }


template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view(cv_mat_qualified_type& mat, cv_mask_mat_qualified_type& mask_mat) :
	base(mat),
	mask_mat_(mask_mat) { }


template<typename Pixel, typename Mask>
void masked_image_view<Pixel, Mask>::reset(const masked_image_view& im) {
	base::reset(im);
	mask_mat_ = im.mask_mat_;
}
	

template<typename Pixel, typename Mask>
auto masked_image_view<Pixel, Mask>::operator=(const masked_image_view& im) -> masked_image_view& {
	base::operator=(im);
	im.mask_mat_.copyTo(mask_mat_);
	return *this;
}


template<typename Pixel, typename Mask>
auto masked_image_view<Pixel, Mask>::operator=(masked_image_view&& im) -> masked_image_view& {
	base::operator=(std::move(im));
	mask_mat_ = im.mask_mat_;
	return *this;
}


template<typename Pixel, typename Mask>
auto masked_image_view<Pixel, Mask>::mask_array_view() const -> mask_ndarray_view_type {
	if(base::is_null()) return mask_ndarray_view_type::null();
	else return to_ndarray_view(mask_mat_);
}



}
