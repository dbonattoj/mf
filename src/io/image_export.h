/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_IMAGE_EXPORT_H_
#define MF_IMAGE_EXPORT_H_

#include <string>
#include <type_traits>
#include "../image/image_view.h"
#include "../color.h"

namespace mf {

void image_export(const image_view<rgb_color>&, const std::string& filename);
void image_export(const masked_image_view<rgb_color>&, const std::string& filename);
void image_export(const masked_image_view<rgb_color>&, const std::string& filename, const rgb_color& background);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_export
	(const masked_image_view<T>&, const std::string& filename, T min_value, T max_value, const rgb_color& background);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_export
	(const masked_image_view<T>&, const std::string& filename, const rgb_color& background);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_export
	(const masked_image_view<T>&, const std::string& filename, T min_value, T max_value);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_export
	(const masked_image_view<T>&, const std::string& filename);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_export
	(const image_view<T>&, const std::string& filename, T min_value, T max_value);

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_export
	(const image_view<T>&, const std::string& filename);

}

#include "image_export.tcc"

#endif
