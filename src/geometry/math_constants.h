#ifndef MF_MATH_CONSTANTS_H_
#define MF_MATH_CONSTANTS_H_

#include "../common.h"

namespace mf {

constexpr real pi = 3.14159265358979323846264338327950288;
constexpr real half_pi = pi / 2.0;
constexpr real quarter_pi = pi / 4.0;
constexpr real two_pi = pi * 2.0;
constexpr real four_pi = pi * 4.0;

constexpr real radiant_per_degree = pi / 180.0;
constexpr real degree_per_radiant = 180.0 / pi;

}

#endif
