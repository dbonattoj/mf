#ifndef MF_COMMON_H_
#define MF_COMMON_H_

#include <cassert>
#include <cstdint>
#include <ostream>

#include "exceptions.h"

namespace mf {

using time_unit = std::int32_t;

class time_span {
private:
	time_unit start_;
	time_unit end_;

public:
	time_span(time_unit start, time_unit end) :
	start_(start), end_(end) {
		assert(end >= start);
	}
	
	time_unit duration() const noexcept { return end_ - start_; }
	time_unit start_time() const noexcept { return start_; }
	time_unit end_time() const noexcept { return end_; }
	
	bool includes(time_span span) const noexcept {
		return (start_ <= span.start_) && (end_ >= span.end_);
	};
	
	friend bool operator==(const time_span& a, const time_span& b) noexcept {
		return (a.start_ == b.start_) && (a.end_ == b.end_);
	}
	friend bool operator!=(const time_span& a, const time_span& b) noexcept {
		return !(a == b);
	}
	friend std::ostream& operator<<(std::ostream& str, const time_span& span) {
		str << '[' << span.start_ << ", " << span.end_ << '[';
		return str;
	}
};

}

#endif
