#ifndef MF_UTILITY_MISC_H_
#define MF_UTILITY_MISC_H_

#include <string>
#include <vector>

namespace mf {

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

}

#include "misc.tcc"

#endif
