#ifndef MF_IMAGE_H_
#define MF_IMAGE_H_

#include "../ndarray/ndarray_view.h"
#include "../opencv.h"

namespace mf {

template<typename Pixel>
class image {
private:
	ndarray_view<2, Pixel> data_;
	cv::Mat_<Pixel> mat_;
};

}

#endif
