namespace mf {

template<typename Pixel>
image_view<Pixel>::image_view() { }


template<typename Pixel>
image_view<Pixel>::image_view(const view_type& vw) :
	mat_(to_opencv(vw)) { }


template<typename Pixel>
image_view<Pixel>::image_view(const cv_mat_type& mat) :
	mat_(mat) { }


template<typename Pixel>
auto image_view<Pixel>::shape() const -> shape_type {
	return make_ndsize(mat_.size[0], mat_.size[1]);
}


template<typename Pixel>
auto image_view<Pixel>::view() const -> view_type {
	return to_ndarray_view(mat_);
}


///////////////


template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view() { }


template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view(const view_type& vw, const mask_view_type& mask_vw) :
	base(vw),
	mask_mat_(to_opencv(mask_vw)) { }


template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view(const cv_mat_type& mat, const cv_mask_mat_type& mask_mat) :
	base(mat),
	mask_mat_(mask_mat) { }


}
