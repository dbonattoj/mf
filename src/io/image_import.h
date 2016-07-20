#ifndef MF_IMAGE_IMPORT_H_
#define MF_IMAGE_IMPORT_H_

#include <string>
#include "../image/image.h"
#include "../color.h"

namespace mf {

image<rgb_color> image_import(const std::string& filename);

}

#endif
