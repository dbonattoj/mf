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

#ifndef MF_NDSPAN_H_
#define MF_NDSPAN_H_

#include <ostream>
#include "../common.h"
#include "ndcoord.h"

namespace mf {

template<std::size_t Dim, typename T> class ndspan_iterator;

/// Cuboid n-dimensional span delimited by two \ref ndcoord vectors.
/** Represents the interval, rectangular, or in general `Dim`-dimensional cuboid region where for all coordinates `c`
 ** inside it and for each dimension `0 <= i < Dim`, one has `start_pos()[i] <= c[i] < end_pos()[i]`.
 ** Can be zero-length on any axis (possibly on all). */
template<std::size_t Dim, typename T = std::ptrdiff_t>
class ndspan {
public:
	using coordinates_type = ndcoord<Dim, T>;
	using shape_type = ndsize<Dim>;
	
	using iterator = ndspan_iterator<Dim, T>;
	
private:
	coordinates_type start_;
	coordinates_type end_;

	bool invariant_() const;

public:
	ndspan() = default;
	ndspan(const ndspan&) = default;
	ndspan(const coordinates_type& start, const coordinates_type& end) :
		start_(start), end_(end) { Assert_crit(invariant_()); }
		
	ndspan& operator=(const ndspan&) = default;
	
	const coordinates_type& start_pos() const { return start_; }
	const coordinates_type& end_pos() const { return end_; }
	void set_start_pos(const coordinates_type& pos) { start_ = pos; Assert_crit(invariant_()); }
	void set_end_pos(const coordinates_type& pos) { end_ = pos; Assert_crit(invariant_()); }

	friend bool operator==(const ndspan& a, const ndspan& b) {
		return (a.start_ == b.start_) && (a.end_ == b.end_);
	}
	friend bool operator!=(const ndspan& a, const ndspan& b) {
		return (a.start_ != b.start_) || (a.end_ != b.end_);
	}
	
	bool includes(const coordinates_type&) const;
	
	bool includes(const ndspan& sub) const;
	bool includes_strict(const ndspan& sub) const;
			
	shape_type shape() const { return end_ - start_; }
	std::size_t size() const { return shape().product(); }

	iterator begin() const ;
	iterator end() const ;
};


template<std::size_t Dim, typename T>
ndspan<Dim, T> make_ndspan(const ndcoord<Dim, T>& start, const ndcoord<Dim, T>& end) {
	return ndspan<Dim, T>(start, end);
}


template<std::size_t Dim, typename T>
ndspan<Dim, T> make_ndspan(const ndcoord<Dim, T>& end) {
	ndcoord<Dim, T> start;
	return ndspan<Dim, T>(start, end);
}


template<std::size_t Dim, typename T>
std::ostream& operator<<(std::ostream& str, const ndspan<Dim, T>& span) {
	str << '[' << span.start_pos() << ", " << span.end_pos() << '[';
	return str;
}


template<std::size_t Dim, typename T>
ndspan<Dim, T> span_intersection(const ndspan<Dim, T>& a, const ndspan<Dim, T>& b);



/// One-dimensional time span.
/** Derived from `ndspan<1, time_unit>.` */
class time_span : public ndspan<1, time_unit> {	
public:
	time_span() = default;
	time_span(const ndspan& span) : ndspan(span) { }
	time_span(time_unit start, time_unit end) :
		ndspan(start, end) { }
	
	time_unit start_time() const { return start_pos().front(); }
	time_unit end_time() const { return end_pos().front(); }
	time_unit duration() const { return size(); }
	
	void set_start_time(time_unit t) { set_start_pos(t); }
	void set_end_time(time_unit t) { set_end_pos(t); }
};


inline std::ostream& operator<<(std::ostream& str, const time_span& span) {
	str << '[' << span.start_time() << ", " << span.end_time() << '[';
	return str;
}

}


#include "ndspan.tcc"

#endif
