#ifndef MF_COMMON_H_
#define MF_COMMON_H_

#include <cassert>
#include <cstdint>
#include <cmath>

#include "ndarray/ndspan.h"
#include "debug.h"
#include "exceptions.h"

namespace mf {


/// Real number type.
using real = std::float_t;


/// Discrete time unit type.
using time_unit = std::ptrdiff_t;


class time_span : public ndspan<1, time_unit> {
	using base = ndspan<1, time_unit>;
	
public:
	time_span() = default;
	time_span(const base& span) : base(span) { }
	time_span(time_unit start, time_unit end) :
		base{start, end} { }
	
	time_unit start_time() const { return base::start_pos().front(); }
	time_unit end_time() const { return base::end_pos().front(); }
	time_unit duration() const { return base::size(); }
};


inline std::ostream& operator<<(std::ostream& str, const time_span& span) {
	str << '[' << span.start_time() << ", " << span.end_time() << '[';
	return str;
}


}

#endif
