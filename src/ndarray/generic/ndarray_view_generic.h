/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_NDARRAY_VIEW_GENERIC_H_
#define MF_NDARRAY_VIEW_GENERIC_H_

#include "frame_format.h"
#include "../ndarray_view.h"
#include "../ndarray_timed_view.h"
#include "../../common.h"
#include "../detail/ndarray_view_fcall.h"
#include <stdexcept>

namespace mf {

/// Generic \ref ndarray_view where lower dimension(s) are type-erased.
/** `ndarray_view_generic<Dim>` is a type-erased view of `ndarray_view<Dim + K, Elem>`, whose lower `K` dimensions must
 ** have default strides (possibly with padding). Elements of `ndarray_view_generic<Dim>` correspond to sections of
 ** `ndarray_view<Dim + K, Elem>` which fix these `K` dimensions, and are called *frames*.
 ** The view is no longer templatized for the dimensionality and element type of the frames. Frame format information
 ** is stored at runtime as \ref frame_format object.
 ** 
 ** Alternately, frames can be *composite*, i.e. contain data from multiple ndarrays of differing dimension and element
 ** types.
 ** 
 ** `Dim` is the *generic dimension*, and `Dim + K` the *concrete dimension*. `K` is the *frame dimension*.
 ** Along the generic dimensions, sectionning and slicing operations can be used and return another (lower-dimensional)
 ** `ndarray_view_generic` with the same frame format information. Conversion to and from concrete `ndarray_view` is
 ** done with the functions \ref to_generic() and \ref from_generic(). */
template<std::size_t Dim, bool Mutable = true>
class ndarray_view_generic : private ndarray_view<Dim + 1, std::conditional_t<Mutable, byte, const byte>> {
	using base = ndarray_view<Dim + 1, std::conditional_t<Mutable, byte, const byte>>;

public:	
	using frame_ptr = std::conditional_t<Mutable, void*, const void*>;

	using pointer = frame_ptr;
	using index_type = std::ptrdiff_t;
	using coordinates_type = ndptrdiff<Dim>;
	using shape_type = ndsize<Dim>;
	using strides_type = ndptrdiff<Dim>;
	using span_type = ndspan<Dim>;
	
	constexpr static std::size_t dimension = Dim;
	
private:
	frame_format format_;

	using fcall_type = detail::ndarray_view_fcall<ndarray_view_generic<Dim, Mutable>, 1>;

protected:
	ndarray_view_generic section_(std::ptrdiff_t dim, std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const {
		Expects(dim < Dim);
		return ndarray_view_generic(format_, base::section_(dim, start, end, step));
	}


public:
	/// \name Construction
	///@{
	ndarray_view_generic() = default;

	ndarray_view_generic(const ndarray_view_generic<Dim, true>& vw) : base(vw), format_(vw.format_) { }
	// if Mutable == false, then this is not the copy constructor
	// --> and then additional, implicit copy constructor gets created

	ndarray_view_generic(const base& vw, const frame_format& frm) : base(vw), format_(frm) { }
	ndarray_view_generic(frame_ptr start, const shape_type&, const strides_type&, const frame_format&);

	static ndarray_view_generic null() { return ndarray_view_generic(); }
	bool is_null() const noexcept { return base::is_null(); }
	explicit operator bool () const noexcept { return ! is_null(); }

	void reset(const ndarray_view_generic& other) noexcept {
		base::reset(other);
		format_ = other.format_;
	}
	void reset() noexcept { reset(null()); }
	
	friend bool same(const ndarray_view_generic& a, const ndarray_view_generic& b) {
		return (a.format() == b.format()) && (base::same(a, b));
	}
	///@}



	/// \name Attributes
	///@{
	pointer start() const { return static_cast<pointer>(base::start()); }
	shape_type shape() const { return head<Dim>(base::shape()); }
	strides_type strides() const { return head<Dim>(base::strides()); }
		
	const frame_format& format() const noexcept { return format_; }
	ndarray_view_generic array_at(std::ptrdiff_t array_index) const;

	static strides_type default_strides(const frame_format& frm, const shape_type& shp) {
		auto base_shp = ndcoord_cat(shp, frm.frame_size());
		auto base_def_str = base::default_strides(base_shp);
		return head<Dim>(base_def_str);
	}
	
	using base::has_default_strides;
	///@}

	
	
	/// \name Deep assignment
	///@{
	ndarray_view_generic& operator=(const ndarray_view_generic& vw) {
		Expects_crit(vw.format() == format());
		base::operator=(vw);
		return *this;
	}
	
	void assign(const ndarray_view_generic& vw) {
		Expects_crit(vw.format() == format());
		base::assign(vw);
	}
	///@}
	
	
	
	/// \name Deep comparison
	///@{
	bool compare(const ndarray_view_generic& vw) const {
		if(vw.format() != format()) return false;
		else return base::compare(vw);
	}
	
	bool operator==(const ndarray_view_generic& vw) const { return compare(vw); }
	bool operator!=(const ndarray_view_generic& vw) const { return ! compare(vw); }
	///@}



	/// \name Indexing
	///@{
	ndarray_view_generic section
	(const coordinates_type& start, const coordinates_type& end, const strides_type& steps = strides_type(1)) const {
		return ndarray_view_generic(format_, base::section(start, end, steps));
	}
	ndarray_view_generic section(const span_type& span, const strides_type& steps = strides_type(1)) const {
		return ndarray_view_generic(format_, base::section(span, steps));
	}
	
	auto slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const {
		return ndarray_view_generic<Dim - 1, Mutable>(format_, base::slice(c, dimension));
	}
	auto operator[](std::ptrdiff_t c) const {
		return ndarray_view_generic<Dim - 1, Mutable>(format_, base::operator[](c));
	}

	fcall_type operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const {
		return ndarray_view_generic(format_, base::operator()(start, end, step));
	}
	fcall_type operator()(std::ptrdiff_t c) const {
		return ndarray_view_generic(format_, base::operator()(c));
	}
	fcall_type operator()() const {
		return ndarray_view_generic(format_, base::operator()());
	}
	///@}
};



/// Cast input `ndarray_view` to generic `ndarray_view_generic` with given dimension.
template<std::size_t Generic_dim, std::size_t Concrete_dim, typename Concrete_elem>
auto to_generic(const ndarray_view<Concrete_dim, Concrete_elem>& vw);



/// Cast input `ndarray_view_generic` to concrete `ndarray_view` with given dimension, frame shape and element type.
template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Generic_dim, bool Generic_mutable>
auto from_generic(
	const ndarray_view_generic<Generic_dim, Generic_mutable>& vw,
	const ndsize<Concrete_dim - Generic_dim>& frame_shape,
	std::ptrdiff_t array_index = 0
);



}


#include "ndarray_view_generic.tcc"

#endif
