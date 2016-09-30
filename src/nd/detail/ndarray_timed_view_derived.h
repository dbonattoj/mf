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

#ifndef MF_NDARRAY_TIMED_VIEW_DERIVED_H_
#define MF_NDARRAY_TIMED_VIEW_DERIVED_H_

#include <stdexcept>
#include "../../common.h"
#include "../ndarray_view.h"
#include "ndarray_view_fcall.h"

namespace mf { namespace detail {


template<typename Base>
class ndarray_timed_view_derived : public Base {
	using base = Base;

private:
	time_unit start_time_;

	using fcall_type = detail::ndarray_view_fcall<ndarray_timed_view_derived, 1>;
	
public:
	using typename base::coordinates_type;
	using typename base::strides_type;
	using typename base::span_type;

	/// \name Construction
	///@{
	ndarray_timed_view_derived() : base(), start_time_(-1) { }

	ndarray_timed_view_derived(const base& vw, time_unit start_time) :
		base(vw), start_time_(vw.is_null() ? -1 : start_time) { }
	
	template<typename... Args> void reset(const Args&... args) {
		reset(ndarray_timed_view_derived(args...));
	}
	void reset(const ndarray_timed_view_derived& vw) {
		start_time_ = vw.start_time_;
		base::reset(vw);
	}

	static ndarray_timed_view_derived null() { return ndarray_timed_view_derived(); }
	
	friend bool same(const ndarray_timed_view_derived& a, const ndarray_timed_view_derived& b) noexcept {
		return (a.start_time() == b.start_time()) && same(a.non_timed(), b.non_timed());
	}
	
	const base& non_timed() const noexcept { return *this; }
	///@}
	
	
	
	/// \name Attributes
	///@{
	time_unit start_time() const { return start_time_; }
	time_unit end_time() const { return start_time_ + base::shape().front(); }
	time_unit duration() const { return base::shape().front(); }
	
	[[deprecated]] std::ptrdiff_t time_index(time_unit t) const { return t - start_time_; }
	[[deprecated]] time_unit time_at(std::ptrdiff_t i) const { return start_time_ + base::fix_coordinate_(i, 0); }
	
	std::ptrdiff_t time_to_index(time_unit t) const { return t - start_time_; }
	time_unit index_to_time(std::ptrdiff_t i) const { return start_time_ + base::fix_coordinate_(i, 0); }

	time_span span() const { return time_span(start_time(), start_time() + duration()); }
	///@}
	
	
	
	/// \name Indexing
	///@{
	decltype(auto) at_time(time_unit t) const { return base::operator[](time_to_index(t)); }
	
	auto time_section(time_span span) {
		std::ptrdiff_t start = time_to_index(span.start_time());
		std::ptrdiff_t end = time_to_index(span.end_time());
		return operator()(start, end);
	}

	auto section
	(const coordinates_type& start, const coordinates_type& end, const strides_type& steps = strides_type(1)) const {
		return ndarray_timed_view_derived(base::section(start, end, steps), start_time_ + start.first());
	}

	auto section(const span_type& span, const strides_type& steps = strides_type(1)) const {
		return section(span.start_pos(), span.end_pos(), steps);
	}
	
	fcall_type operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const {
		return ndarray_timed_view_derived(base::section_(0, start, end, step), start_time_ + start);
	}
	fcall_type operator()(std::ptrdiff_t c) const {
		return ndarray_timed_view_derived(base::section_(0, c, c + 1, 1), start_time_ + c);
	}
	fcall_type operator()() const {
		return *this;
	}
	///@}
};

}}

#endif
