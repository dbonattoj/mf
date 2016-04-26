#ifndef MF_COLOR_H_
#define MF_COLOR_H_

#include <cstdint>
#include "elem.h"

namespace mf {

struct mono_color {
	mono_color() = default;
	mono_color(std::uint8_t nint) : intensity(nint) { }
	
	mono_color(const mono_color&) = default;
	mono_color& operator=(const mono_color&) = default;
	
	std::uint8_t intensity;

	const static mono_color black;
	const static mono_color white;
};

bool operator==(const mono_color& a, const mono_color& b);
bool operator!=(const mono_color& a, const mono_color& b);


struct rgb_color {
	rgb_color() = default;
	rgb_color(std::uint8_t nr, std::uint8_t ng, std::uint8_t nb) :
		r(nr), g(ng), b(nb) { }
	
	rgb_color(const rgb_color&) = default;
	rgb_color& operator=(const rgb_color&) = default;
	
	
	std::uint8_t r; // red
	std::uint8_t g; // green
	std::uint8_t b; // blue
	
	const static rgb_color black;
	const static rgb_color white;
};

bool operator==(const rgb_color& a, const rgb_color& b);
bool operator!=(const rgb_color& a, const rgb_color& b);



struct rgba_color {
	rgba_color() = default;
	rgba_color(std::uint8_t nr, std::uint8_t ng, std::uint8_t nb, std::uint8_t na = 255) :
		r(nr), g(ng), b(nb), a(na) { }

	rgba_color(const rgba_color&) = default;
	rgba_color& operator=(const rgba_color&) = default;

	std::uint8_t r; // red
	std::uint8_t g; // green
	std::uint8_t b; // blue
	std::uint8_t a; // alpha
	
	static rgba_color null() noexcept { return rgba_color(0, 0, 0, 0); }
	bool is_null() const noexcept { return (a == 0); }
	
	const static rgba_color black;
	const static rgba_color white;
};

bool operator==(const rgba_color& a, const rgba_color& b);
bool operator!=(const rgba_color& a, const rgba_color& b);


struct ycbcr_color {
	ycbcr_color() = default;
	ycbcr_color(std::uint8_t ny, std::uint8_t ncr, std::uint8_t ncb) :
		y(ny), cr(ncr), cb(ncb) { }
	
	ycbcr_color(const ycbcr_color&) = default;
	ycbcr_color& operator=(const ycbcr_color&) = default;

	std::uint8_t y;  // luma, Y'
	std::uint8_t cr; // chroma-red, U
	std::uint8_t cb; // chroma-blue, V
};

bool operator==(const ycbcr_color& a, const ycbcr_color& b);
bool operator!=(const ycbcr_color& a, const ycbcr_color& b);




template<typename Output, typename Input>
Output color_convert(const Input&);

template<> rgb_color color_convert(const ycbcr_color&);

template<> inline rgba_color color_convert(const rgb_color& in) {
	return rgba_color(in.r, in.g, in.b, 255);
}

template<> inline rgb_color color_convert(const mono_color& in) {
	return rgb_color(in.intensity, in.intensity, in.intensity);
}

template<> inline mono_color color_convert(const ycbcr_color& in) {
	return mono_color(in.y);
}

template<> inline rgba_color color_convert(const ycbcr_color& in) {
	return color_convert<rgba_color>(color_convert<rgb_color>(in));
}

// TODO remove, replace by thin/implicit nodes
template<> inline std::uint8_t color_convert(const ycbcr_color& in) {
	return in.y;
}


template<> inline rgb_color color_convert(const rgba_color& in) {
	return rgb_color(in.r, in.g, in.b);
}


template<>
struct elem_traits<mono_color> :
	elem_traits_base<mono_color, std::uint8_t, 1, false> { };

template<>
struct elem_traits<rgb_color> :
	elem_traits_base<rgb_color, std::uint8_t, 3, false> { };
	
template<>
struct elem_traits<rgba_color> :
	elem_traits_base<rgba_color, std::uint8_t, 4, true> { };

template<>
struct elem_traits<ycbcr_color> :
	elem_traits_base<ycbcr_color, std::uint8_t, 3, false> { };



}

#endif
