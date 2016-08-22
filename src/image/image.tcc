namespace mf {

template<typename Pixel>
image<Pixel>::image(const shape_type& shape) :
	mat_(shape[0], shape[1]) { }


template<typename Pixel>
image<Pixel>::image(const cv_mat_type& mat) :
	mat_()
{
	mat.copyTo(mat_);
}


template<typename Pixel>
image<Pixel>::image(const const_ndarray_view_type& ndvw) :
	mat_()
{
	copy_to_opencv(ndvw, mat_);
}


template<typename Pixel>
image<Pixel>::image(const const_view_type& vw) :
	mat_()
{
	vw.cv_mat().copyTo(mat_);
}


template<typename Pixel>
image<Pixel>::image(image&& im) :
	mat_(im.mat_) { }


template<typename Pixel>
auto image<Pixel>::operator=(const const_ndarray_view_type& ndvw) -> image& {
	copy_to_opencv(ndvw, mat_);
	return *this;
}


template<typename Pixel>
auto image<Pixel>::operator=(const const_view_type& vw) -> image& {
	vw.cv_mat().copyTo(mat_);
	return *this;
}


template<typename Pixel>
auto image<Pixel>::operator=(image&& im) -> image& {
	mat_ = im.mat_;
	return *this;
}


template<typename Pixel>
auto image<Pixel>::shape() const -> shape_type {
	return shape_type(mat_.size[0], mat_.size[1]);
}


}
