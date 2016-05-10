#ifndef MF_NDARRAY_VIEW_GENERIC_H_
#define MF_NDARRAY_VIEW_GENERIC_H_

#include "frame_format.h"
#include "../ndarray_view.h"
#include "../ndarray_timed_view.h"
#include "../../common.h"
#include <stdexcept>

namespace mf {

/// Generic \ref ndarray_view where lower dimension(s) are type-erased.
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

	ndarray_view_generic() : format_(frame_format::null()) { }
	
public:	
	ndarray_view_generic(const base& vw, const frame_format& format) :
		base(vw), format_(format) { }
	
	ndarray_view_generic(byte* start, const frame_format& format, const shape_type& shape, const strides_type& strides) :
		base(start, shape, strides),
		format_(format) { }

	static ndarray_view_generic null() { return ndarray_view_generic(); }
	
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
	
	void reset(const ndarray_view_generic& other) noexcept {
		base::reset(other);
		format_ = other.format_;
	}
	void reset() noexcept { reset(null()); }
};



/// Cast input `ndarray_view` to generic `ndarray_view_generic` with given dimension.
/** `Generic_dim` is dimension of generic ndarray, i.e. `Dim` of `ndarray_view_generic<Dim>`.
 ** `Concrete_dim` is dimension of the typed ndarray, i.e. `Dim` of `ndarray_view<Dim>`.
 ** Always `Concrete_dim >= Generic_dim`. Dimension of frame is `Concrete_dim - Generic_dim`. */
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
