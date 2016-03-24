#ifndef MF_COLOR_H_
#define MF_COLOR_H_

#include <cstdint>

namespace mf {

struct mono_color {
	std::uint8_t intensity;
};


struct rgb_color {
	std::uint8_t r; // red
	std::uint8_t g; // green
	std::uint8_t b; // blue
};


struct ycbcr_color {
	std::uint8_t y;  // luma, Y'
	std::uint8_t cr; // chroma-red, U
	std::uint8_t cb; // chroma-blue, V
};


template<typename Input, typename Output>
Output color_convert(const Input& in);


template<> rgb_color color_convert(const ycbcr_color& in);
template<> mono_color color_convert(const ycbcr_color& in);
template<> rgb_color color_convert(const mono_color& in);

}

#endif
