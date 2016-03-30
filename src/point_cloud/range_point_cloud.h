#ifndef MF_RANGE_POINT_CLOUD_H_
#define MF_RANGE_POINT_CLOUD_H_

#include "point_cloud.h"

namespace mf {

template<typename Point>
class range_point_cloud : public point_cloud<Point> {
	using base = point_cloud<Point>;

private:
	ndarray_view<2, Point> view_;

public:
	explicit range_point_cloud(const ndarray_view<2, Point>& arr) :
		base(arr.reshape(make_ndsize(arr.size()))), view_(arr) { }
};

}

#endif
