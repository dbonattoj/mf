#ifndef MF_COLOR_H_
#define MF_COLOR_H_

#include <cstdint>
#include "elem.h"

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


template<typename Output, typename Input>
Output color_convert(const Input& in);


template<> rgb_color color_convert(const ycbcr_color& in);
template<> mono_color color_convert(const ycbcr_color& in);
template<> rgb_color color_convert(const mono_color& in);


template<>
struct elem_traits<mono_color> :
	elem_traits_base<mono_color, std::uint8_t, 1> { };

template<>
struct elem_traits<rgb_color> :
	elem_traits_base<rgb_color, std::uint8_t, 3> { };

template<>
struct elem_traits<ycbcr_color> :
	elem_traits_base<ycbcr_color, std::uint8_t, 3> { };



}

#endif
