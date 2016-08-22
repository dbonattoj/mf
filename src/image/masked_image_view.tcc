namespace mf {

template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view() { }


template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view(const masked_image_view& im) :
	base(im),
	mask_mat_(im.mask_mat_) { }


template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view(const ndarray_view_type& vw, const mask_ndarray_view_type& mask_vw) :
	base(vw),
	mask_mat_(to_opencv(mask_vw))
{
	Assert(vw.shape() == mask_vw.shape(), "image and mask must have same shape");
}


template<typename Pixel, typename Mask>
masked_image_view<Pixel, Mask>::masked_image_view(cv_mat_qualified_type& mat, cv_mask_mat_qualified_type& mask_mat) :
	base(mat),
	mask_mat_(mask_mat)
{
	Assert(mat.size == mask_mat.size, "image and mask must have same shape");
}


template<typename Pixel, typename Mask>
void masked_image_view<Pixel, Mask>::reset(const masked_image_view& im) {
	base::reset(im);
	mask_mat_ = im.mask_mat_;
}


template<typename Pixel, typename Mask>
auto masked_image_view<Pixel, Mask>::mask_array_view() const -> mask_ndarray_view_type {
	if(base::is_null()) return mask_ndarray_view_type::null();
	else return to_ndarray_view(mask_mat_);
}

}
