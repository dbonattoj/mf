/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
