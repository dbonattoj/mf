#include "color.h"
#include "utility/misc.h"

namespace mf {

template<> rgb_color color_convert(const ycbcr_color& in) {
	float y  = static_cast<float>(in.y) + 0.5f;
	float cr = static_cast<float>(in.cr) - 127.0f;
	float cb = static_cast<float>(in.cb) - 127.0f;

	float r = clamp(y             + 1.402f*cb, 0.0f, 255.0f);
	float g = clamp(y - 0.344f*cr - 0.714f*cb, 0.0f, 255.0f);
	float b = clamp(y + 1.772f*cr            , 0.0f, 255.0f);
	
	return { static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b) };
}


template<> mono_color color_convert(const ycbcr_color& in) {
	return { in.y };
}


template<> rgb_color color_convert(const mono_color& in) {
	return { in.intensity, in.intensity, in.intensity };
}


}
