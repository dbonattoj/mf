#ifndef MF_COLOR_H_
#define MF_COLOR_H_

#include <cstdint>
#include "util.h"

namespace mf {

struct rgb_color {
	std::uint8_t red;
	std::uint8_t green;
	std::uint8_t blue;
};


struct ycbcr_color {
	std::uint8_t y; // luma, Y'
	std::uint8_t cr; // chroma-red, U
	std::uint8_t cb; // chroma-blue, V
};


template<typename Input, typename Output>
Output color_convert(const Input& in);


template<>
rgb_color color_convert(const ycbcr_color& in) {
	float y  = static_cast<float>(in.y) + 0.5f;
	float cr = static_cast<float>(in.cr) - 127.0f;
	float cb = static_cast<float>(in.cb) - 127.0f;

	float r = clamp(y             + 1.402f*cb, 0.0f, 255.0f);
	float g = clamp(y - 0.344f*cr - 0.714f*cb, 0.0f, 255.0f);
	float b = clamp(y + 1.772f*cr            , 0.0f, 255.0f);
	
	return { static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b) };
}


}

#endif
