#ifndef MF_NDARRAY_GENERIC_H_
#define MF_NDARRAY_GENERIC_H_

#include "ndarray.h"

namespace mf {

/// Array container with type erased multidimensional frames.
/** Like \ref ndarray_view_generic, it retains \ref frame_format with element type and alignment of frames.  */
template<typename Allocator = raw_allocator>
class ndarray_generic : public ndarray<2, byte, Allocator> {
	using base = ndarray<2, byte, Allocator>;

private:
	frame_format format_;

public:
	using view_type = ndarray_view_generic<2>;

	ndarray_generic(
		std::size_t length,
		std::size_t padding,
		const frame_format& format,
		std::size_t frame_length,
		const Allocator& allocator = Allocator()
	)
	: base(
		make_ndsize(length, frame_length),
		padding,
		format.alignment(),
		allocator
	),
	format_(format) { }

	explicit ndarray_generic(const ndarray_generic&) = default;
	
	view_type view() { return  }
};


}

#endif
