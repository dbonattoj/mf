#ifndef MF_COMMON_H_
#define MF_COMMON_H_

#include <cassert>
#include <cstdint>

#include "ndspan.h"
#include "debug.h"
#include "exceptions.h"

namespace mf {

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


}

#endif
