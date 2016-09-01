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

#ifndef MF_NDARRAY_VIEW_OPAQUE_H_
#define MF_NDARRAY_VIEW_OPAQUE_H_

#include "../ndarray_view.h"
#include "../ndarray_timed_view.h"
#include "../../common.h"
#include "../detail/ndarray_view_fcall.h"
#include "../opaque_format/opaque_format.h"
#include "../../utility/misc.h"
#include <stdexcept>
#include <memory>
#include <utility>

namespace mf {

/// \ref ndarray_view where opaque frames with runtime-determined size take the place of elements.
/** Frames are opaque segments with runtime-determined size and alignment requirement, and their format is defined with
 ** \ref ndarray_opaque_frame_format. With multi-part format, the \ref ndarray_view_opaque can be casted into one that
 ** covers only one part, using extract_part().
 ** With single-part format, the \ref ndarray_view_opaque is a type-erased \ref ndarray_view: It can be casted to/from
 ** a concrete \ref ndarray_view using from_opaque() and to_opaque(). */
template<std::size_t Dim, bool Mutable = true>
class ndarray_view_opaque : private ndarray_view<Dim + 1, std::conditional_t<Mutable, byte, const byte>> {
	using base_value_type = std::conditional_t<Mutable, byte, const byte>;
	using base = ndarray_view<Dim + 1, base_value_type>;

public:	
	using frame_ptr = std::conditional_t<Mutable, void*, const void*>;
	using pointer = frame_ptr;
	using index_type = std::ptrdiff_t;
	using coordinates_type = ndptrdiff<Dim>;
	using shape_type = ndsize<Dim>;
	using strides_type = ndptrdiff<Dim>;
	using span_type = ndspan<Dim>;
	
	using format_base_type = opaque_format;
	using format_ptr = std::shared_ptr<const format_base_type>;

	constexpr static std::size_t dimension = Dim;
	
private:
	format_ptr frame_format_;

	using fcall_type = detail::ndarray_view_fcall<ndarray_view_opaque<Dim, Mutable>, 1>;

protected:
	ndarray_view_opaque section_(std::ptrdiff_t dim, std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const;
	using base::fix_coordinate_;

public:
	/// \name Construction
	///@{
	ndarray_view_opaque() = default;
	
	ndarray_view_opaque(const ndarray_view_opaque<Dim, true>& vw) :
		base(vw.base_view()), frame_format_(vw.frame_format_ptr()) { }
	// if Mutable == false, then this is not the copy constructor
	// --> and then additional, implicit copy constructor gets created

	ndarray_view_opaque(const base& vw, const format_ptr& frm) :
		base(vw), frame_format_(frm) { }

	ndarray_view_opaque(frame_ptr start, const shape_type&, const strides_type&, const format_ptr&);

	template<typename Format, typename = enable_if_derived_from_opaque_format<Format>>
	ndarray_view_opaque(const base& vw, Format&& frm) :
		ndarray_view_opaque(vw, forward_make_shared(frm)) { }
		
	template<typename Format, typename = enable_if_derived_from_opaque_format<Format>>
	ndarray_view_opaque(frame_ptr start, const shape_type& shp, const strides_type& str, Format&& frm) :
		ndarray_view_opaque(start, shp, str, forward_make_shared_const(frm)) { }

	static ndarray_view_opaque null() { return ndarray_view_opaque(); }
	bool is_null() const noexcept { return base::is_null(); }
	explicit operator bool () const noexcept { return ! is_null(); }

	template<typename... Args> void reset(const Args&... args) {
		reset(ndarray_view_opaque(args...));
	}
	void reset(const ndarray_view_opaque& other) noexcept {
		base::reset(other.base_view());
		frame_format_ = other.frame_format_;
	}
	
	const base& base_view() const { return *this; }
	///@}



	/// \name Attributes
	///@{
	pointer start() const { return static_cast<pointer>(base::start()); }
	shape_type shape() const { return head<Dim>(base::shape()); }
	strides_type strides() const { return head<Dim>(base::strides()); }

	std::size_t size() const { return shape().product(); }
	span_type full_span() const noexcept { return span_type(0, shape()); }
		
	static strides_type default_strides(const shape_type&, const format_base_type&, std::size_t frame_pad = 0);	
	bool has_default_strides(std::ptrdiff_t minimal_dimension = 0) const noexcept;
	std::size_t default_strides_padding(std::ptrdiff_t minimal_dimension = 0) const;
	bool has_default_strides_without_padding(std::ptrdiff_t minimal_dimension = 0) const noexcept;

	const format_ptr& frame_format_ptr() const noexcept { return frame_format_; }
	const format_base_type& frame_format() const noexcept { Assert(frame_format_); return *frame_format_; }
	///@}

	
	
	/// \name Deep assignment
	///@{
	const ndarray_view_opaque& operator=(const ndarray_view_opaque& vw) const { assign(vw); return *this; }
	
	void assign(const ndarray_view_opaque<Dim, false>& vw) const;
	///@}
	
	
	
	/// \name Deep comparison
	///@{
	bool compare(const ndarray_view_opaque& vw) const;
	
	bool operator==(const ndarray_view_opaque& vw) const { return compare(vw); }
	bool operator!=(const ndarray_view_opaque& vw) const { return ! compare(vw); }
	///@}



	/// \name Indexing
	///@{
	ndarray_view_opaque<0, Mutable> at(const coordinates_type&) const;
	
	ndarray_view_opaque section
	(const coordinates_type& start, const coordinates_type& end, const strides_type& steps = strides_type(1)) const {
		return ndarray_view_opaque(base::section(start, end, steps), frame_format_);
	}
	ndarray_view_opaque section(const span_type& span, const strides_type& steps = strides_type(1)) const {
		return ndarray_view_opaque(base::section(span, steps), frame_format_);
	}
	
	auto slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const {
		return ndarray_view_opaque<Dim - 1, Mutable>(base::slice(c, dimension), frame_format_);
	}
	auto operator[](std::ptrdiff_t c) const {
		return ndarray_view_opaque<Dim - 1, Mutable>(base::operator[](c), frame_format_);
	}

	fcall_type operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const {
		return ndarray_view_opaque(base::operator()(start, end, step), frame_format_);
	}
	fcall_type operator()(std::ptrdiff_t c) const {
		return ndarray_view_opaque(base::operator()(c), frame_format_);
	}
	fcall_type operator()() const {
		return ndarray_view_opaque(base::operator()(), frame_format_);
	}
	///@}
};


template<std::size_t Dim, bool Mutable1, bool Mutable2>
bool same(const ndarray_view_opaque<Dim, Mutable1>& a, const ndarray_view_opaque<Dim, Mutable2>& b);


/// Cast from \ref ndarray_view to opaque \ref ndarray_view_opaque with given dimension.
/** Opaque view gets \ref opque_ndarray_format format. */
template<std::size_t Opaque_dim, std::size_t Concrete_dim, typename Concrete_elem>
auto to_opaque(const ndarray_view<Concrete_dim, Concrete_elem>& concrete_view);



/// Cast from \ref ndarray_view_opaque to concrete \ref ndarray_view with given dimension, frame shape and element type.
/** Opaque view needs to have format with array. */
template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Opaque_dim, bool Opaque_mutable>
auto from_opaque(
	const ndarray_view_opaque<Opaque_dim, Opaque_mutable>& opaque_view,
	const ndsize<Concrete_dim - Opaque_dim>& frame_shape
);


/// Cast from \ref ndarray_view_opaque to single-array view to one part.
/** Opaque view needs to have format with parts. */
template<std::size_t Dim, bool Mutable>
auto extract_part(const ndarray_view_opaque<Dim, Mutable>& vw, std::ptrdiff_t part_index);



}

#include "ndarray_view_opaque.tcc"

#endif
