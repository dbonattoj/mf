#ifndef MF_UTILITY_MISC_H_
#define MF_UTILITY_MISC_H_

#include <string>
#include <vector>

namespace mf {

bool file_exists(const std::string& filepath);

template<typename Numeric>
Numeric sq(Numeric n) { return n * n; }

template<typename Input, typename Output = Input>
Output* advance_raw_ptr(Input* ptr, std::ptrdiff_t diff) noexcept;

template<typename T>
T clamp(T value, T minimum, T maximum);

}

#include "misc.tcc"

#endif
