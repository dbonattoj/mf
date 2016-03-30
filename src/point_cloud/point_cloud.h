#ifndef MF_POINT_CLOUD_H_
#define MF_POINT_CLOUD_H_

#include "../ndarray/ndarray_view.h"

namespace mf {

template<class Point>
class point_cloud {
private:
	ndarray_view<1, Point> view_;

public:
	explicit point_cloud(const ndarray_view<1, Point>& arr) :
		view_(arr) { }
};

}

#endif