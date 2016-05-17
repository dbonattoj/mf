#ifndef PROG_COMMON_H_
#define PROG_COMMON_H_

#include <cstdint>
#include <mf/masked_elem.h>
#include <mf/color.h>

namespace mf {

template<> inline std::uint8_t color_convert(const mf::ycbcr_color& in) {
	return in.y;
}

template<> inline mf::masked_elem<std::uint8_t> color_convert(const mf::ycbcr_color& in) {
	if(in.y == 0.0) return mf::masked_elem<std::uint8_t>::null();
	else return in.y;
}

template<> inline mf::rgb_color color_convert(const std::uint8_t& in) {
	return mf::rgb_color(in, in, in);
}

template<> inline mf::rgb_color color_convert(const mf::masked_elem<mf::rgb_color>& in) {
	if(in.is_null()) return mf::rgb_color(0, 0, 0); // background
	else return in;
}

template<> inline mf::rgb_color color_convert(const mf::masked_elem<std::uint8_t>& in) {
	if(in.is_null()) return mf::rgb_color(0, 0, 0); // background
	else return mf::rgb_color(in, in, in);
}

}

using depth_type = std::uint8_t;

#endif
