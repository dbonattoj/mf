#ifndef MF_RING_H_
#define MF_RING_H_

#include <functional>
#include <mutex>
#include <ostream>
#include "../common.h"
#include "ndcoord.h"
#include "ndarray.h"
#include "../utility/memory.h"

namespace mf {

/// Ring buffer ndarray where first dimension is circular.
/** Adds additional first _time_ dimension, the underlying `ndarray` has `Dim+1` dimensions. A _frame_ is a time slice,
 ** i.e. `ring[t]`. Array wraps around at the end in time dimension, i.e. `ring[t+total_duration()]` would be
 ** equivalent to `ring[t]`. (But subscript operator is still limited to array bounds).
 ** Provides first-in, last-out ring buffer interface to write and read frames in the array. It gives `ndarray_view`
 ** views to write in / read from, which may internally wrap around.
 ** Wrapping is implemented using virtual memory mapping: Address contiguity is preserved and no special view subclass
 ** is needed. */
class ring : public ndarray<Dim + 1, T, raw_ring_allocator> {
	using base = ndarray<Dim + 1, T, raw_ring_allocator>;

public:
	using section_view_type = ndarray_view<Dim + 1, T>;

private:
	time_unit read_position_ = 0;
	time_unit write_position_ = 0;
	bool full_ = false;
		
	static std::size_t adjust_padding_(const ndsize<Dim>& spatial_shape, std::size_t duration); 
	section_view_type section_(time_unit start, time_unit duration);

public:
	ndarray_ring(const ndsize<Dim>& frames_shape, time_unit duration);
	
	void initialize();
	
	time_unit total_duration() const noexcept { return base::shape().front(); }
	
	time_unit writable_duration() const;
	time_unit readable_duration() const;
	
	section_view_type begin_write(time_unit duration);
	void end_write(time_unit written_duration);
	
	section_view_type begin_read(time_unit duration);
	void end_read(time_unit read_duration);
		
	void skip(time_unit duration);
	
	#ifndef NDEBUG
	void debug_print(std::ostream&) const;
	#endif
};

}

#include "ndarray_ring.tcc"

#endif
