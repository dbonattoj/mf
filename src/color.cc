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

	float r = clamp(y             + 1.402f*cr, 0.0f, 255.0f);
	float g = clamp(y - 0.344f*cb - 0.714f*cr, 0.0f, 255.0f);
	float b = clamp(y + 1.772f*cb            , 0.0f, 255.0f);
		
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
