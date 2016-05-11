#ifndef MF_NDARRAY_FRAME_FORMAT_H_
#define MF_NDARRAY_FRAME_FORMAT_H_

#include "../../common.h"
#include "../../utility/misc.h"
#include <typeinfo>
#include <typeindex>

namespace mf {

/// Format information of type-erased frame of `ndarray_view_generic`.
/** Stores element type of anterior `ndarray_view` (`std::type_info`, `sizeof` and `alignof`), and the actual alignment
 ** of elements in the frame. */
class frame_format {
private:
	std::type_index elem_type_ = std::type_index(typeid(void)); ///< Type of element.
	std::size_t elem_size_ = 0; ///< sizeof of element.
	std::size_t elem_alignment_ = 0; ///< alignof of elememt.
	
	/// Stride of element in frame.
	/** Must be greater or equal to `elem_size_`, and non-zero multiple of `elem_alignment_requirement_`. 
	 ** Corresponds to stride of elements in the frame. */
	std::size_t stride_ = 0;

	frame_format() = default;

public:
	frame_format(const frame_format&) = default;
	frame_format& operator=(const frame_format&) = default;

	static frame_format null() { return frame_format(); }

	template<typename Elem>
	static frame_format default_format(std::size_t stride = sizeof(Elem)) {
		static_assert(sizeof(Elem) >= alignof(Elem), "sizeof always larger or equal to alignof");
		MF_EXPECTS(stride >= sizeof(Elem) && is_nonzero_multiple_of(stride, alignof(Elem)));
		frame_format format;
		format.elem_type_ = std::type_index(typeid(Elem));
		format.elem_size_ = sizeof(Elem);
		format.elem_alignment_ = alignof(Elem);
		format.stride_ = stride;
		return format;
	}
		
	template<typename Elem>
	bool is_type() const noexcept {
		std::type_index query_type(typeid(Elem));
		return (elem_type_ == query_type);
	}
	
	std::type_index elem_type_index() const noexcept { return elem_type_; }
	std::size_t elem_size() const noexcept { return elem_size_; }
	std::size_t elem_alignment() const noexcept { return elem_alignment_; }
	std::size_t stride() const noexcept { return stride_; }
	std::size_t padding() const noexcept { return (stride_ - elem_size_); }
};


}

#endif

