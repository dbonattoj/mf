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

#ifndef MF_UTILITY_MISC_H_
#define MF_UTILITY_MISC_H_

#include <string>
#include <vector>

namespace mf {
	
template<typename...> using void_t = void;

/// Check if a file at `filename` exists.
bool file_exists(const std::string& filename);

/// Compute square of a number.
template<typename Numeric>
Numeric sq(Numeric n) { return n * n; }

/// Advance a pointer `ptr` of any type by `diff` bytes.
/** Pointer may be `void*` or any other pointer type. Does not verify alignment. */
template<typename T>
T* advance_raw_ptr(T* ptr, std::ptrdiff_t diff) noexcept;

/// Clamp `value` between `minimum` and `maximum` value.
template<typename T>
T clamp(T value, T minimum, T maximum);

/// Compute greatest common divisor of `a` and `b`.
template<typename T>
T gcd(T a, T b);

/// Check if `x` is a power of 2.
template<typename T>
bool is_power_of_two(T x) {
	return (x != 0) && !(x & (x - 1));
}

/// Check if `x` is a multiple of `base`, including zero.
template<typename T>
bool is_multiple_of(T x, T base) {
	return (x % base == 0);
}

/// Check if `x` is a non-zero multiple of `base`.
template<typename T>
bool is_nonzero_multiple_of(T x, T base) {
	return (x != 0) && is_multiple_of(x, base);
}

/// Check if `x` is odd.
template<typename T>
bool is_odd(T x) { return (x % 2) != 0; }

/// Check if `x` is even.
template<typename T>
bool is_even(T x) { return (x % 2) == 0; }

}

#include "misc.tcc"

#endif
