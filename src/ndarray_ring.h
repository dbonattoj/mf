#ifndef MF_NDARRAY_RING_H_
#define MF_NDARRAY_RING_H_

#include <functional>
#include "common.h"
#include "ndcoord.h"
#include "ndarray.h"
#include "ring_allocator.h"

namespace mf {

/// Ring buffer ndarray where first dimension is circular.
/** Adds additional first _time_ dimension, the underlying `ndarray` has `Dim+1` dimensions. A _frame_ is a time slice,
 ** i.e. `ring[t]`. Array wraps around at the end in time dimension, i.e. `ring[t+total_duration()]` would be
 ** equivalent to `ring[t]`. (But subscript operator is still limited to array bounds).
 ** Provides first-in, last-out ring buffer interface to write and read frames in the array. It gives `ndarray_view`
 ** views to write in / read from, which may internally wrap around.
 ** Wrapping is implemented using virtual memory mapping: Address contiguity is preserved and no special view subclass
 ** is needed. */
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
