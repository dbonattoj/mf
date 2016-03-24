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
