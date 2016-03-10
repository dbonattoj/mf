#ifndef MF_NDARRAY_RING_H_
#define MF_NDARRAY_RING_H_

#include <functional>
#include "common.h"
#include "ndcoord.h"
#include "ndarray.h"
#include "ring_allocator.h"

namespace mf {

/// Ndarray where first dimension is circular (ring buffer).
template<std::size_t Dim, typename T>
class ndarray_ring : public ndarray<Dim + 1, T, ring_allocator<T>> {
	using base = ndarray<Dim + 1, T, ring_allocator<T>>;

public:
	using section_view_type = ndarray_view<Dim + 1, T>;

	using typename base::padding_type;

private:
	std::size_t read_position_ = 0;
	std::size_t write_position_ = 0;
	bool full_ = false;
	
	static padding_type adjust_padding_(const ndsize<Dim>& spatial_shape, std::size_t duration); 
	section_view_type section_(std::ptrdiff_t start, std::size_t duration);

public:
	ndarray_ring(const ndsize<Dim>& frames_shape, std::size_t duration);
	
	std::size_t total_duration() const noexcept { return base::shape().front(); }
	
	std::size_t writable_duration() const;
	std::size_t readable_duration() const;
	
	virtual section_view_type begin_write(std::size_t duration);
	virtual void end_write(std::size_t written_duration);
	
	virtual section_view_type begin_read(std::size_t duration);
	virtual void end_read(std::size_t read_duration);
		
	virtual void skip(std::size_t duration);
};

}

#include "ndarray_ring.tcc"

#endif
