#ifndef MF_NDSPAN_H_
#define MF_NDSPAN_H_

#include <ostream>
#include "../common.h"
#include "ndcoord.h"

namespace mf {


template<std::size_t Dim, typename T>
class ndspan {
public:
	using coordinates_type = ndcoord<Dim, T>;
	using shape_type = ndcoord<Dim, T>;
	
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
};


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