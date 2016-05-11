#ifndef MF_IMAGE_KERNEL_H_
#define MF_IMAGE_KERNEL_H_

#include "../common.h"
#include "../ndarray/ndarray.h"

namespace mf {

using bool_image_kernel = ndarray<2, bool>;
using real_image_kernel = ndarray<2, real>;

bool_image_kernel disk_image_kernel(std::size_t diameter);
bool_image_kernel box_image_kernel(std::size_t diameter);

}

#endif
