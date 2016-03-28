#ifndef MF_POINT_CLOUD_H_
#define MF_POINT_CLOUD_H_

#include "../ndarray/ndarray_view.h"

namespace mf {

template<class Point>
class point_cloud : public ndarray_view<1, Point> {

};

}

#endif