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
image_view<Pixel>::image_view(const image_view& im) :
	mat_(im.cv_mat()) { }

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


}
