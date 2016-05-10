#ifndef MF_NDARRAY_GENERIC_H_
#define MF_NDARRAY_GENERIC_H_

#include "frame_format.h"
#include "ndarray_view_generic.h"
#include "../ndarray.h"
#include "../../elem.h"
#include "../../utility/memory.h"

namespace mf {

/// Properties for construction of \ref ndarray_generic and derived container objects.
/** Described frame format, frame length and number of frames. Does not describe padding between array frames, because
 ** this may be determined internally by the container. (For example, \ref ring needs to adjust is to fit page size.) */
struct ndarray_generic_properties {
	frame_format format_; ///< Element type and alignment of frame.
	std::size_t frame_length_; ///< Number of elements in one frame.
	std::size_t array_length_; ///< Number of frames.
	
	const frame_format& format() const noexcept { return format_; }
	std::size_t frame_length() const noexcept { return frame_length_; }
	std::size_t array_length() const noexcept { return array_length_; }
	std::size_t frame_size() const noexcept { return frame_length_ * format_.alignment(); }

	ndarray_generic_properties(const frame_format& format, std::size_t frame_length, std::size_t array_length) :
		format_(format), frame_length_(frame_length), array_length_(array_length) { }
};


/// Array container with type erased multidimensional frames.
/** Like \ref ndarray_view_generic, it retains \ref frame_format with element type and alignment of frames.  */
template<typename Allocator = raw_allocator>
class ndarray_generic : public ndarray<2, byte, Allocator> {
	using base = ndarray<2, byte, Allocator>;

private:
	frame_format format_;

public:
	using view_type = ndarray_view_generic<2>;

	ndarray_generic(const ndarray_generic_properties&, std::size_t padding = 0, const Allocator& = Allocator());
	explicit ndarray_generic(const ndarray_generic&) = default;

	const frame_format& format() const noexcept { return format_; }

	view_type view() { return view_type(base::view(), format_); }	
};


}

#include "ndarray_generic.tcc"

#endif
