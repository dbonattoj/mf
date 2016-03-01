#ifndef MF_IMAGE_H_
#define MF_IMAGE_H_

#include "ndarray_view.h"

namespace mf {

template<typename Color>
class image : public ndarray_view<2, Color> {

};

}

#endif