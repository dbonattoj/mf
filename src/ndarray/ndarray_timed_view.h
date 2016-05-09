#ifndef MF_NDARRAY_TIMED_VIEW_H_
#define MF_NDARRAY_TIMED_VIEW_H_

#include <stdexcept>
#include "../common.h"
#include "ndarray_view.h"

namespace mf {

/// Ndarray view with absolute time indices associated to first dimension.
template<std::size_t Dim, typename T>
class ndarray_timed_view : public ndarray_view<Dim, T> {
	using base = ndarray_view<Dim, T>;

private:
	time_unit start_time_;
	
public:
	ndarray_timed_view() : base(), start_time_(-1) { }

	explicit ndarray_timed_view(time_unit start_time) :
		base(), start_time_(start_time) { }

	ndarray_timed_view(const base& vw, time_unit start_time) :
		base(vw), start_time_(start_time) { }
	
	static ndarray_timed_view null() { return ndarray_timed_view(); }

	time_unit start_time() const { return start_time_; }
	time_unit end_time() const { return start_time_ + base::shape().front(); }
	time_unit duration() const { return base::shape().front(); }
	
	std::ptrdiff_t time_index(time_unit t) const { return t - start_time_; }
	time_unit time_at(std::ptrdiff_t i) const { return start_time_ + base::fix_coordinate_(i, 0); }
	
	decltype(auto) at_time(time_unit t) const {
		if(span().includes(t)) return base::operator[](time_index(t));
		else throw std::out_of_range("time out of bounds");
	}
	decltype(auto) at_time(time_unit t) {
		if(span().includes(t)) return base::operator[](time_index(t));
		else throw std::out_of_range("time out of bounds");
	}

	time_span span() const { return time_span(start_time(), start_time() + duration()); }
	
	void reset(const ndarray_timed_view& vw) {
		start_time_ = vw.start_time_;
		base::reset(vw);
	}
};
// TODO section() etc.

}

#endif
