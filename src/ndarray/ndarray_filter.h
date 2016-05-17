#ifndef MF_NDARRAY_FILTER_H_
#define MF_NDARRAY_FILTER_H_

#include "ndarray_view.h"

namespace mf {

/// Placement of a kernel on an `ndarray_view`.
template<std::size_t Dim, typename Elem, typename Kernel_elem>
struct kernel_placement {
	ndarray_view<Dim, Elem> view_section; ///< Section of the view covered by kernel.
	ndarray_view<Dim, Kernel_elem> kernel_section; ///< Section of the kernel which covers view (may be truncated).
	ndptrdiff<Dim> section_position; ///< Coordinates of kernel center in `view_section` and `kernel_section`.
	ndptrdiff<Dim> absolute_position; ///< Coordinates of kernel center in full view.
};


/// Return \ref kernel_placement object for placement of kernel `kernel` over `view` centered at `pos`.
/** Kernel shape must be odd in all axis. When `pos` is near limits of view span, the sections get truncated. */
template<std::size_t Dim, typename Elem, typename Kernel_elem>
kernel_placement<Dim, Elem, Kernel_elem> place_kernel_at(
	const ndarray_view<Dim, Elem>& view,
	const ndarray_view<Dim, Kernel_elem> kernel,
	const ndptrdiff<Dim>& pos
);


/// Place `kernel` over all elements in `in_view`, and call `func` to modify corresponding `out_view` element.
/** \param func Function `void(const kernel_placement<Dim, In_elem, Kernel_elem>&, Out_elem&)`.
 ** \param in_view View over which kernel gets applied. `In_elem` may be const.
 ** \param out_view View whose elements get modified by kernel. Must be same shape as `in_view`.
 ** \param kernel The kernel. */
template<std::size_t Dim, typename In_elem, typename Out_elem, typename Kernel_elem, typename Function>
void apply_kernel(
	Function&& func,
	const ndarray_view<Dim, In_elem>& in_view,
	const ndarray_view<Dim, Out_elem>& out_view,
	const ndarray_view<Dim, Kernel_elem> kernel
);


}

#include "ndarray_filter.tcc"

#endif
