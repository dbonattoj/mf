namespace mf {

template<typename Depth>
bool depth_image_camera<Depth>::in_depth_bounds(real d) const {
	return (d >= pixel_depth_minimum_) && (d <= pixel_depth_maximum_);
}


template<typename Depth>
auto depth_image_camera<Depth>::to_pixel_depth(real d) const -> pixel_depth_type {
	real diff_f = static_cast<real>(pixel_depth_maximum_ - pixel_depth_minimum_);
	real pixd_f = (d - depth_origin_) * diff_f / depth_range_;
	return static_cast<pixel_depth_type>(pixd_f);
}


template<typename Depth>
auto depth_image_camera<Depth>::to_pixel_depth_clamp(real d) const -> pixel_depth_type {
	// TODO more efficient clamp, no branch
	real diff_f = static_cast<real>(pixel_depth_maximum_ - pixel_depth_minimum_);
	real pixd_f = (d - depth_origin_) * diff_f / depth_range_;
	if(pixd_f <= pixel_depth_minimum_)
		return static_cast<pixel_depth_type>(pixel_depth_minimum_);
	else if(pixd_f >= pixel_depth_maximum_)
		return static_cast<pixel_depth_type>(pixel_depth_maximum_);
	else
		return static_cast<pixel_depth_type>(pixd_f);
}


template<typename Depth>
real depth_image_camera<Depth>::to_depth(pixel_depth_type pixd) const {
	real diff_f = static_cast<real>(pixel_depth_maximum_ - pixel_depth_minimum_);
	real pixd_f = static_cast<real>(pixd);
	return (pixd_f * depth_range_ / diff_f) + depth_origin_;
}


}
