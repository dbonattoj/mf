#ifndef MF_IMAGE_H_
#define MF_IMAGE_H_

#include "../ndarray/ndarray_view.h"
#include "../opencv.h"
#include "../opencv_ndarray.h"

namespace mf {

template<typename Pixel>
class image {
public:
	using view_type = ndarray_view<2, Pixel>;
	using cv_mat_type = cv::Mat_<Pixel>;

private:
	ndarray_view<2, Pixel> view_;
	cv_mat_type mat_;

public:
	image(const view_type& vw) :
		view_(vw), mat_(to_opencv_mat(vw)) { }
	
	const cv_mat_type& cv_mat() const { return mat_; }
	cv_mat_type& cv_mat() { return mat_; }
};

}

#endif
