#ifndef MF_NDARRAY_VIEW_GENERIC_H_
#define MF_NDARRAY_VIEW_GENERIC_H_

#include "ndarray_view.h"
#include "ndarray_timed_view.h"
#include "../common.h"
#include <typeinfo>
#include <typeindex>
#include <stdexcept>

namespace mf {

// TODO make less verbose, remove code duplication

/// Format information of type-erased frame of `ndarray_view_generic`.
/** Stores element type of anterior `ndarray_view` (`std::type_info`, `sizeof` and `alignof`), and the actual alignment
 ** of elements in the frame. */
class frame_format {
private:
	std::type_index elem_type_;
	std::size_t elem_size_ = 0;
	std::size_t elem_alignment_requirement_ = 0;
	
	std::size_t alignment_ = 0;

	frame_format() : elem_type_(typeid(void)) { }

public:
	template<typename Elem>
	static frame_format default_format(std::size_t alignment = alignof(Elem)) {
		frame_format format;
		format.elem_type_ = std::type_index(typeid(Elem));
		format.elem_size_ = sizeof(Elem);
		format.elem_alignment_requirement_ = alignof(Elem);
		format.alignment_ = alignment;
		return format;
	}
	
	template<typename Elem>
	bool is_type() const noexcept {
		std::type_index query_type(typeid(Elem));
		return (elem_type_ == query_type);
	}
	
	std::type_index type_index() const noexcept { return elem_type_; }
	std::size_t elem_size() const noexcept { return elem_size_; }
	std::size_t alignment() const noexcept { return alignment_; }
	std::size_t padding() const noexcept { return (alignment_ - elem_size_); }
};


/// Generic ndarray_view where lower dimension(s) are type-erased.
/** Elements of `ndarray_view_generic<Dim>` represent entire `k`-dimensional sections (called _frame_) of an anterior
 ** `ndarray_view<Dim + k, Elem>`. This `ndarray_view` must have default strides (possibly with padding)
 ** for the first `k` dimensions. `ndarray_view_generic` preserves type information of `T` (`std::type_info`, along with
 ** its `sizeof` and `alignof`), its actual alignment (last stride of anterior `ndarray_view`), and number of elements
 ** per frame. Dimension and shape of frame is not preserved.
 **
 ** Generic ndarray view `ndarray_view_generic<Dim>` is internally represented as `ndarray_view<Dim + 1, byte>`. The
 ** last axis corresponds to the type-erased `k`-dimensional frame, represented as raw byte array. Its shape and stride
 ** are the number and alignment of `T` elements in the frame.
 **
 ** Subscripting results in another `ndarray_view_generic` of lower dimension. */
template<std::size_t Dim>
class ndarray_view_generic : public ndarray_view<Dim + 1, byte> {
	using base = ndarray_view<Dim + 1, byte>;

public:
	using shape_type = typename base::shape_type;
	using strides_type = typename base::strides_type;
	
private:
	frame_format format_;
	
public:
	ndarray_view_generic() = default;
	ndarray_view_generic(const base& vw, const frame_format& format) :
		base(vw), format_(format) { }
	ndarray_view_generic(byte* start, const frame_format& format, const shape_type& shape, const strides_type& strides) :
		base(start, shape, strides),
		format_(format) { }
	
	const frame_format& format() const noexcept { return format_; }
	
	decltype(auto) slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const
		{ return ndarray_view_generic<Dim - 1>(base::slice(c, dimension), format_); }

	decltype(auto) operator[](std::ptrdiff_t c) const
		{ return ndarray_view_generic<Dim - 1>(base::operator[](c), format_); }

	decltype(auto) operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const
		{ return ndarray_view_generic(base::operator()(start, end, step), format_); }
		
	decltype(auto) operator()(std::ptrdiff_t c) const
		{ return ndarray_view_generic(base::operator()(c), format_); }

	decltype(auto) operator()() const
		{ return ndarray_view_generic(base::operator()(), format_); }
};



/// Cast input `ndarray_view` to generic `ndarray_view_generic` with given dimension.
template<std::size_t Generic_dim, std::size_t Concrete_dim, typename Concrete_elem>
ndarray_view_generic<Generic_dim> to_generic(const ndarray_view<Concrete_dim, Concrete_elem>& vw);



/// Cast input `ndarray_view_generic` to concrete `ndarray_view` with given dimension, frame shape and element type.
template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Generic_dim>
ndarray_view<Concrete_dim, Concrete_elem> from_generic(
	const ndarray_view_generic<Generic_dim>& vw,
	const ndsize<Concrete_dim - Generic_dim>& frame_shape
);



}


#include "ndarray_view_generic.tcc"

#endif
