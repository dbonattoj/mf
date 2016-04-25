#include <sstream>
#include <typeinfo>
#include <algorithm>

namespace mf {


template<typename T>
inline T* advance_raw_ptr(T* ptr, std::ptrdiff_t diff) noexcept {
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


template<typename T>
T gcd(T a, T b) {
	if(a < b) std::swap(a, b);
	while(b > 0) {
		T c = a % b;
		a = b;
		b = c;
	}
	return a;
}


}
