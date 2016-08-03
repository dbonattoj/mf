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

#ifndef MF_NDARRAY_TIMED_VIEW_H_
#define MF_NDARRAY_TIMED_VIEW_H_

#include <stdexcept>
#include "../common.h"
#include "ndarray_view.h"

namespace mf {
	
namespace detail {
	template<std::size_t Dim, typename T, std::ptrdiff_t Target_dim>
	class ndarray_timed_view_fcall;
}


/// Ndarray view with absolute time indices associated to first dimension.
/** Each frame `vw[i]` is associated with time index `t = start_time + i`. */
template<std::size_t Dim, typename T>
class ndarray_timed_view : public ndarray_view<Dim, T> {
	using base = ndarray_view<Dim, T>;

private:
	time_unit start_time_;
	
public:
	using typename base::coordinates_type;
	using typename base::strides_type;
	using typename base::span_type;

	static ndarray_timed_view null() { return ndarray_timed_view(); }

	/// Create null timed view.
	ndarray_timed_view() : base(), start_time_(-1) { }

	/// Create timed view where time index `start_time` is associated with frame `vw[0]`.
	/** `vw` cannot be the null view. */
	ndarray_timed_view(const base& vw, time_unit start_time) :
		base(vw), start_time_(start_time)
	{
		Expects(! vw.is_null());	
	}
	
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
	
	ndarray_timed_view section(
		const coordinates_type& start,
		const coordinates_type& end,
		const strides_type& steps = strides_type(1)
	) const {
		return ndarray_timed_view(base::section(start, end, steps), start_time_ + start.first());
	}

	ndarray_timed_view section(const span_type& span, const strides_type& steps = strides_type(1)) const {
		return section(span.start_pos(), span.end_pos(), steps);
	}
	
	using fcall_result = detail::ndarray_timed_view_fcall<Dim, T, 1>;
	fcall_result operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const {
		return ndarray_timed_view(base::section_(0, start, end, step), start_time_ + start);
	}
	fcall_result operator()(std::ptrdiff_t c) const {
		return ndarray_timed_view(base::section_(0, c, c + 1, 1), start_time_ + c);
	}
	fcall_result operator()() const {
		return *this;
	}

	time_span span() const { return time_span(start_time(), start_time() + duration()); }
	
	void reset(const ndarray_timed_view& vw) {
		start_time_ = vw.start_time_;
		base::reset(vw);
	}
};

}

#include "ndarray_timed_view.tcc"

#endif
