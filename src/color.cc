#include "color.h"
#include "utility/misc.h"

namespace mf {
	
const mono_color mono_color::black(0);
const mono_color mono_color::white(255);


const rgb_color rgb_color::black(0, 0, 0);
const rgb_color rgb_color::white(255, 255, 255);


const rgba_color rgba_color::black(0, 0, 0);
const rgba_color rgba_color::white(255, 255, 255);


template<>
rgb_color color_convert(const ycbcr_color& in) {
	float y  = static_cast<float>(in.y) + 0.5f;

	float cr = static_cast<float>(in.cr) - 127.0f;
	float cb = static_cast<float>(in.cb) - 127.0f;

	float r = clamp(y             + 1.402f*cb, 0.0f, 255.0f);
	float g = clamp(y - 0.344f*cr - 0.714f*cb, 0.0f, 255.0f);
	float b = clamp(y + 1.772f*cr            , 0.0f, 255.0f);
		
	return rgb_color(static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b));
}


bool operator==(const mono_color& a, const mono_color& b) {
	return (a.intensity == b.intensity);
}

bool operator!=(const mono_color& a, const mono_color& b) {
	return (a.intensity != b.intensity);
}

bool operator==(const rgb_color& a, const rgb_color& b) {
	return (a.r == b.r) && (a.g == b.g) && (a.b == b.b);
}

bool operator!=(const rgb_color& a, const rgb_color& b) {
	return (a.r != b.r) || (a.g != b.g) || (a.b != b.b);
}

bool operator==(const rgba_color& a, const rgba_color& b) {
	return (a.r == b.r) && (a.g == b.g) && (a.b == b.b) && (a.a == b.a);
}

bool operator!=(const rgba_color& a, const rgba_color& b) {
	return (a.r != b.r) || (a.g != b.g) || (a.b != b.b) || (a.a != b.a);
}

bool operator==(const ycbcr_color& a, const ycbcr_color& b) {
	return (a.y == b.y) && (a.cr == b.cr) && (a.cb == b.cb);
}

bool operator!=(const ycbcr_color& a, const ycbcr_color& b) {
	return (a.y != b.y) || (a.cr != b.cr) || (a.cb != b.cb);
}


}
