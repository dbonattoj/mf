#ifndef MF_UTILITY_MISC_H_
#define MF_UTILITY_MISC_H_

#include <string>
#include <vector>

namespace mf {

bool file_exists(const std::string& filepath);

template<typename Numeric>
Numeric sq(Numeric n) { return n * n; }

template<typename T>
T* advance_raw_ptr(T* ptr, std::ptrdiff_t diff) noexcept;

template<typename T>
T clamp(T value, T minimum, T maximum);

template<typename T>
T gcd(T a, T b);

template<typename T>
bool is_power_of_two(T x) {
	return (x != 0) && !(x & (x - 1));
}

}

#include "misc.tcc"

#endif
