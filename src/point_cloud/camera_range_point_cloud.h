#include "range_point_cloud.h"

namespace mf {

template<typename Point, typename Image_camera>
class camera_range_point_cloud : public range_point_cloud<Point> {
	using base = range_point_cloud<Point>;

private:
	Image_camera camera_;

public:
	camera_range_point_cloud(const ndarray_view<2, Point>& arr, const Image_camera& cam) :
		base(arr), camera_(cam) { }
};

}