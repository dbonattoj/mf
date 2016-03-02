#ifndef MF_UTIL_H_
#define MF_UTIL_H_

#include <cstddef>
#include <cassert>
#include <cstdint>
#include <type_traits>

namespace mf {

template<typename T>
T* advance_raw_ptr(T* ptr, std::ptrdiff_t diff) noexcept {
	std::uintptr_t raw_ptr = reinterpret_cast<std::uintptr_t>(ptr);
	raw_ptr += diff;
	return reinterpret_cast<T*>(raw_ptr);
}


template<typename T>
inline T clamp(T value, T minimum, T maximum) {
	if(value > maximum) value = maximum;
	else if(value < minimum) value = minimum;
	return value;
}


}

#endif
