#ifndef MF_TESTSUPPORT_NDARRAY_H_
#define MF_TESTSUPPORT_NDARRAY_H_

#include "../../src/ndarray/ndarray.h"

namespace mf {

ndarray<2, int> make_frame(const ndsize<2>& shape, int i);

}

#endif
