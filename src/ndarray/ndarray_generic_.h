#ifndef MF_NDARRAY_GENERIC_H_
#define MF_NDARRAY_GENERIC_H_

#include "../common.h"
#include "ndarray.h"
#include "ndarray_view.h"
#include "ndarray_timed_view.h"
#include "../elem.h"
#include "../utility/memory.h"
#include "../utility/misc.h"
#include <cstdint>

namespace mf {

using ndarray_view_generic = ndarray_view<2, byte>;
using ndarray_timed_view_generic = ndarray_timed_view<2, byte>;
using ndarray_frame_view_generic = ndarray_view<1, byte>;


template<std::size_t Dim, typename Elem>
ndarray_frame_view_generic to_generic_frame(const ndarray_view<Dim, Elem>&);

template<std::size_t Dim, typename Elem>
ndarray_view<Dim, Elem> from_generic_frame(const ndarray_frame_view_generic&, const ndsize<Dim>&);

template<std::size_t Dim, typename Elem>
ndarray_view_generic to_generic(const ndarray_view<Dim + 1, Elem>&);

template<std::size_t Dim, typename Elem>
ndarray_timed_view_generic to_generic_timed(const ndarray_timed_view<Dim + 1, Elem>&);

template<std::size_t Dim, typename Elem>
ndarray_view<Dim + 1, Elem> from_generic(const ndarray_view_generic&, const ndsize<Dim>& frame_shape);

template<std::size_t Dim, typename Elem>
ndarray_timed_view<Dim + 1, Elem> from_generic_timed(const ndarray_timed_view_generic&, const ndsize<Dim>& frame_shape);



class frame_format {
private:
	std::size_t elem_size_ = 0; ///< Size of element, including padding if any.
	std::size_t alignment_ = 0; ///< Alignment requirement of elements and of whole frame.

public:
	frame_format() = default;
	
	frame_format(std::size_t elem_size, std::size_t alignment) :
		elem_size_(elem_size), alignment_(alignment)
	{
		MF_EXPECTS(elem_size > 0);
		MF_EXPECTS(alignment > 0 && is_power_of_two(alignment));
		MF_EXPECTS(elem_size != 0 && (elem_size % alignment == 0));
	}
	
	template<typename Elem>
	static frame_format default_format() {
		return frame_format(sizeof(Elem), alignof(Elem));
	}
	
	static frame_format common(const frame_format& a, const frame_format& b) {
		return frame_format(
			std::max(a.elem_size(), b.elem_size()),
			std::max(a.alignment(), b.alignment())			
		);
	}
	
	std::size_t elem_size() const noexcept { return elem_size_; }
	std::size_t alignment() const noexcept { return alignment_; }
};


struct frame_array_properties {
	frame_format format; ///< Element type and alignment of frame.
	std::size_t frame_length = 0; ///< Number of elements in one frame.
	std::size_t array_length = 0; ///< Number of frames.
	
	frame_array_properties() = default;
	frame_array_properties(const frame_format& form, std::size_t fr_len, std::size_t arr_len) :
		format(form), frame_length(fr_len), array_length(arr_len)
	{
		MF_EXPECTS(array_length > 0);
	}


	frame_array_properties(const frame_array_properties&) = default;
	frame_array_properties& operator=(const frame_array_properties&) = default;
	
	std::size_t frame_size() const noexcept { return format.elem_size() * frame_length; }
	std::size_t elem_size() const noexcept { return format.elem_size(); }
	std::size_t alignment() const noexcept { return format.alignment(); }
};


template<typename Allocator = raw_allocator>
class ndarray_generic : public ndarray<2, byte, Allocator> {
	using base = ndarray<2, byte, Allocator>;
	
public:
	ndarray_generic(const frame_array_properties&, std::size_t frame_padding = 0, const Allocator& = Allocator());
	explicit ndarray_generic(const ndarray_generic&) = default;
};


}

#include "ndarray_generic.tcc"

#endif
