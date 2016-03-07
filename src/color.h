#ifndef MF_COLOR_H_
#define MF_COLOR_H_

#include <cstdint>
#include "util.h"

namespace mf {

struct alignas(4) rgb_color {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
};


struct alignas(4) ycbcr_color {
	std::uint8_t y; // luma, Y'
	std::uint8_t cr; // chroma-red, U
	std::uint8_t cb; // chroma-blue, V
};


template<typename Input, typename Output>
Output color_convert(const Input& in);


template<> rgb_color color_convert(const ycbcr_color& in);

}

#endif
