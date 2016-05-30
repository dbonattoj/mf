#ifndef MF_IMAGE_WRITE_H_
#define MF_IMAGE_WRITE_H_

#include <string>
#include <type_traits>
#include "image.h"
#include "../color.h"

namespace mf {

void image_write(const image<rgb_color>&, const std::string& filename);
void image_write(const masked_image<rgb_color>&, const std::string& filename);
void image_write(const masked_image<rgb_color>&, const std::string& filename, const rgb_color& background);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const masked_image<T>&, const std::string& filename, T min_value, T max_value, const rgb_color& background);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const masked_image<T>&, const std::string& filename, const rgb_color& background);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const masked_image<T>&, const std::string& filename, T min_value, T max_value);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const masked_image<T>&, const std::string& filename);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const image<T>&, const std::string& filename, T min_value, T max_value);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const image<T>&, const std::string& filename);

}

#include "image_write.tcc"

#endif
