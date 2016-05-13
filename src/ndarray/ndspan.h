#ifndef MF_NDSPAN_H_
#define MF_NDSPAN_H_

#include <ostream>
#include "../common.h"
#include "ndcoord.h"

namespace mf {

/// Cuboid n-dimensional span delimited by two `ndcoord` vectors.
/** Represents the interval, rectangular, or in general `Dim`-dimensional cuboid region where for all coordinates `c`
 ** inside it and for each dimension `0 <= i < Dim`, one has `start_pos()[i] <= c[i] < end_pos()[i]`.
 ** Can be zero-length on any axis (possibly on all). */
template<std::size_t Dim, typename T = std::ptrdiff_t>
class ndspan {
public:
	using coordinates_type = ndcoord<Dim, T>;
	using shape_type = ndsize<Dim>;
	
private:
	coordinates_type start_;
	coordinates_type end_;

public:
	ndspan() = default;
	ndspan(const ndspan&) = default;
	ndspan(const coordinates_type& start, const coordinates_type& end);
		
	ndspan& operator=(const ndspan&) noexcept = default;
	
	const coordinates_type& start_pos() const noexcept { return start_; }
	const coordinates_type& end_pos() const noexcept { return end_; }

	friend bool operator==(const ndspan& a, const ndspan& b) noexcept {
		return (a.start_ == b.start_) && (a.end_ == b.end_);
	}
	friend bool operator!=(const ndspan& a, const ndspan& b) noexcept {
		return (a.start_ != b.start_) || (a.end_ != b.end_);
	}
	
	bool includes(const coordinates_type&) const;
	
	bool includes(const ndspan& sub) const;
	bool includes_strict(const ndspan& sub) const;
			
	shape_type shape() const { return end_ - start_; }
	std::size_t size() const { return shape().product(); }

	// TODO iterator over span coordinates
};


template<std::size_t Dim, typename T>
ndspan<Dim, T> make_ndspan(const ndcoord<Dim, T>& start, const ndcoord<Dim, T>& end) {
	return ndspan<Dim, T>(start, end);
}


template<std::size_t Dim, typename T>
std::ostream& operator<<(std::ostream& str, const ndspan<Dim, T>& span) {
	str << '[' << span.start_pos() << ", " << span.end_pos() << '[';
	return str;
}


template<std::size_t Dim, typename T>
ndspan<Dim, T> span_intersection(const ndspan<Dim, T>& a, const ndspan<Dim, T>& b);

}

#include "ndspan.tcc"

#endif
