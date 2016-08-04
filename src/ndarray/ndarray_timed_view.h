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

/// \ref ndarray_view with absolute time indices associated to first dimension.
/** Each frame `vw[i]` is associated with time index `t = start_time + i`. */
template<typename Base>
class ndarray_timed_view_base : public Base {
	using base = Base;

private:
	time_unit start_time_;
	
public:
	using typename base::coordinates_type;
	using typename base::strides_type;
	using typename base::span_type;

	static ndarray_timed_view_base null() { return ndarray_timed_view_base(); }
	ndarray_timed_view_base() : base(), start_time_(-1) { }

	ndarray_timed_view_base(const base& vw, time_unit start_time) :
		base(vw), start_time_(vw.is_null() ? -1 : start_time) { }
	
	const base& non_timed() const noexcept { return *this; }
	
	time_unit start_time() const { return start_time_; }
	time_unit end_time() const { return start_time_ + base::shape().front(); }
	time_unit duration() const { return base::shape().front(); }
	
	std::ptrdiff_t time_index(time_unit t) const { return t - start_time_; }
	time_unit time_at(std::ptrdiff_t i) const { return start_time_ + base::fix_coordinate_(i, 0); }
	
	std::ptrdiff_t time_to_index(time_unit t) const { return t - start_time_; }
	time_unit index_to_time(std::ptrdiff_t i) const { return start_time_ + base::fix_coordinate_(i, 0); }
	
	decltype(auto) at_time(time_unit t) const { return base::operator[](time_index(t)); }
	decltype(auto) at_time(time_unit t) { return base::operator[](time_index(t)); }
	
	auto section(
		const coordinates_type& start,
		const coordinates_type& end,
		const strides_type& steps = strides_type(1)
	) const {
		return ndarray_timed_view_base(base::section(start, end, steps), start_time_ + start.first());
	}

	auto section(const span_type& span, const strides_type& steps = strides_type(1)) const {
		return section(span.start_pos(), span.end_pos(), steps);
	}
	
	using fcall_type = detail::ndarray_view_fcall<ndarray_timed_view_base<Base>, 1>;
	fcall_type operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const {
		return ndarray_timed_view_base(base::section_(0, start, end, step), start_time_ + start);
	}
	fcall_type operator()(std::ptrdiff_t c) const {
		return ndarray_timed_view_base(base::section_(0, c, c + 1, 1), start_time_ + c);
	}
	fcall_type operator()() const {
		return *this;
	}
	
	time_span span() const { return time_span(start_time(), start_time() + duration()); }
	
	void reset(const ndarray_timed_view_base& vw) {
		start_time_ = vw.start_time_;
		base::reset(vw);
	}
};

template<std::size_t Dim, typename T>
using ndarray_timed_view = ndarray_timed_view_base<ndarray_view<Dim, T>>;

}

#endif
