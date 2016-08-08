#ifndef MF_NDARRAY_GENERIC_H_
#define MF_NDARRAY_GENERIC_H_

#include "ndarray_view_generic.h"
#include "../detail/ndarray_wrapper.h"
#inculde "../os/memory.h"

namespace mf {

/// Container for \ref ndarray_view_generic.
template<std::size_t Dim, typename Allocator = raw_allocator>
class ndarray_generic :
public detail::ndarray_wrapper<ndarray_view_generic<Dim, true>, ndarray_view_generic<Dim, false>, Allocator> {
	using base = detail::ndarray_wrapper<ndarray_view_generic<Dim, true>, ndarray_view_generic<Dim, false>, Allocator>;
	
public:
	using typename base::view_type;
	using typename base::const_view_type;
	using typename base::shape_type;
	using typename base::strides_type;
	
	/// \name Constructor
	///@{
	/// Construct null \ref ndarray_generic.
	explicit ndarray_generic(const Allocator& = Allocator());
	
	/// Construct empty \ref ndarray_generic with given shape and framt format.
	/** Has default strides, optionally with specified frame padding. */
	ndarray_generic
	(const shape_type& shape, const frame_format& frm, std::size_t frame_padding = 0, const Allocator& = Allocator());
	
	/// Construct \ref ndarray_generic with shape and copy of elements from a \ref ndarray_view_generic.
	/** Has default strides, optionally with specified frame padding. Does not take strides from \a vw. */
	explicit ndarray_generic(const const_view_type& vw, std::size_t frame_padding = 0, const Allocator& = Allocator());
	
	/// Copy-construct from another \ref ndarray_generic of same type.
	/** Takes strides from \a arr. */
	ndarray_generic(const ndarray_generic& arr);
	
	/// Move-construct from another \ref ndarray_generic of same type.
	/** Takes strides from \a arr and sets \a arr to null. */
	ndarray_generic(ndarray_generic&& arr);
	///@}
	

	/// \name Deep assignment
	///@{
	/// Assign shape and elements from \ref vw.
	/** Resets to default strides, optionally with specified frame padding. Reallocates memory if necessary. */
	void assign(const const_view_type& vw, std::size_t frame_padding = 0);
	
	/// Assign shape and elements from \ref vw.
	/** Equivalent to `assign(vw)`. */
	ndarray_generic& operator=(const const_view_type& vw)
		{ assign(vw); return *this; }

	/// Copy-assign from another \ref ndarray_generic.
	/** Takes strides from \a arr */
	ndarray_generic& operator=(const ndarray_generic& arr);

	/// Move-assign from another \ref ndarray_generic.
	/** Takes strides from \a arr and sets \a arr to null. */
	ndarray_generic& operator=(ndarray_generic&& arr);
	///@}
	
	
	/// \name Attributes
	///@{
	const frame_format& format() const noexcept { return base:view().format(); }
	view_type array_at(std::ptrdiff_t array_index) { return base::view().array_at(array_index); }
	const_view_type array_at(std::ptrdiff_t array_index) const { return base::cview().array_at(array_index); }
	///@}
}

}

#include "ndarray_generic.tcc"

#endif
