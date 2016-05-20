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

#ifndef MF_ANGLE_H_
#define MF_ANGLE_H_

#include <iosfwd>
#include <cmath>
#include "math_constants.h"

namespace mf {

/// Angle, represented in radiants.
/** Defines I/O in degrees, and conversion. Provides unambiguity in what angles mean. */
struct angle {
private:
	float radiants_;

public:
	static angle degrees(float d) { return d * radiant_per_degree; }
	static angle radiants(float r) { return r; }	

	angle() = default;
	angle(float r) : radiants_(r) { }
	angle(const angle&) = default;
	
	const float& get_radiants() const { return radiants_; }
	float get_degrees() const { return radiants_ * degree_per_radiant; }
	void set_radiants(float r) { radiants_ = r; }
	void set_degrees(float d) { radiants_ = d * radiant_per_degree; }
	
	operator float& () { return radiants_; }
	operator const float& () const { return radiants_; }
	
	angle& operator=(float r) { radiants_ = r; return *this; }
	angle& operator=(const angle&) = default;
	
	bool operator==(angle a) const { return radiants_ == a.radiants_; }
	bool operator!=(angle a) const { return radiants_ != a.radiants_; }
	
	static angle smallest_between(angle a, angle b);
};

inline angle operator"" _deg(long double deg) {
	return angle::degrees(deg);
}

inline angle operator"" _rad(long double rad) {
	return angle::radiants(rad);
}

std::ostream& operator<<(std::ostream&, angle);
std::istream& operator>>(std::istream&, angle&);

}

#endif
