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

#include "opaque_format_array.h"
#include "../ndarray_view.h"
#include "../ndarray_timed_view.h"
#include "../../common.h"
#include "../detail/ndarray_view_fcall.h"
#include <stdexcept>

namespace mf {


/// \ref ndarray_view where opaque frames with runtime-determined size take the place of elements.
/** Frames are opaque segments with runtime-determined size and alignment requirement, and their format is defined with
 ** \ref ndarray_opaque_frame_format. With multi-part format, the \ref ndarray_view_opaque can be casted into one that
 ** covers only one part, using extract_part().
 ** With single-part format, the \ref ndarray_view_opaque is a type-erased \ref ndarray_view: It can be casted to/from
 ** a concrete \ref ndarray_view using from_opaque() and to_opaque(). */
template<std::size_t Dim, typename Format = opaque_format_array, bool Mutable = true>
class ndarray_view_opaque : private ndarray_view<Dim + 1, std::conditional_t<Mutable, byte, const byte>> {
	using base_value_type = std::conditional_t<Mutable, byte, const byte>;
	using base = ndarray_view<Dim + 1, base_value_type>;

public:	
	using format_type = Format;

	using frame_ptr = std::conditional_t<
		Mutable,
		typename format_type::frame_ptr,
		typename format_type::const_frame_ptr
	>;

	using pointer = frame_ptr;
	using index_type = std::ptrdiff_t;
	using coordinates_type = ndptrdiff<Dim>;
	using shape_type = ndsize<Dim>;
	using strides_type = ndptrdiff<Dim>;
	using span_type = ndspan<Dim>;
	
	constexpr static std::size_t dimension = Dim;
	
private:
	format_type format_;

	using fcall_type = detail::ndarray_view_fcall<ndarray_view_opaque<Dim, format_type, Mutable>, 1>;

protected:
	ndarray_view_opaque section_(std::ptrdiff_t dim, std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const;
	using base::fix_coordinate_;

public:
	/// \name Construction
	///@{
	ndarray_view_opaque() = default;

	ndarray_view_opaque(const ndarray_view_opaque<Dim, format_type, true>& vw) :
		base(vw.base_view()), format_(vw.format()) { }
	// if Mutable == false, then this is not the copy constructor
	// --> and then additional, implicit copy constructor gets created

	ndarray_view_opaque(const base& vw, const format_type& frm) : base(vw), format_(frm) { }
	ndarray_view_opaque(frame_ptr start, const shape_type&, const strides_type&, const format_type&);

	static ndarray_view_opaque null() { return ndarray_view_opaque(); }
	bool is_null() const noexcept { return base::is_null(); }
	explicit operator bool () const noexcept { return ! is_null(); }


	template<typename... Args> void reset(const Args&... args) {
		reset(ndarray_view_opaque(args...));
	}
	void reset(const ndarray_view_opaque& other) noexcept {
		base::reset(other.base_view());
		format_ = other.format_;
	}
	
	friend bool same(const ndarray_view_opaque& a, const ndarray_view_opaque& b) {
		return (a.format() == b.format()) && (base::same(a, b));
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
		
	static strides_type default_strides(const shape_type&, const ndarray_opaque_frame_format&, std::size_t frame_pad = 0);	
	bool has_default_strides(std::ptrdiff_t minimal_dimension = 0) const noexcept;
	std::size_t default_strides_padding(std::ptrdiff_t minimal_dimension = 0) const;
	bool has_default_strides_without_padding(std::ptrdiff_t minimal_dimension = 0) const noexcept;

	const format_type& frame_format() const noexcept { return format_; }
	[[deprecated]] const format_type& format() const noexcept { return format_; }
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
	ndarray_view_opaque<0, format_type, Mutable> at(const coordinates_type&) const;
	
	ndarray_view_opaque section
	(const coordinates_type& start, const coordinates_type& end, const strides_type& steps = strides_type(1)) const {
		return ndarray_view_opaque(format_, base::section(start, end, steps));
	}
	ndarray_view_opaque section(const span_type& span, const strides_type& steps = strides_type(1)) const {
		return ndarray_view_opaque(format_, base::section(span, steps));
	}
	
	auto slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const {
		return ndarray_view_opaque<Dim - 1, format_type, Mutable>(base::slice(c, dimension), format_);
	}
	auto operator[](std::ptrdiff_t c) const {
		return ndarray_view_opaque<Dim - 1, format_type, Mutable>(base::operator[](c), format_);
	}

	fcall_type operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const {
		return ndarray_view_opaque(base::operator()(start, end, step), format_);
	}
	fcall_type operator()(std::ptrdiff_t c) const {
		return ndarray_view_opaque(base::operator()(c), format_);
	}
	fcall_type operator()() const {
		return ndarray_view_opaque(base::operator()(), format_);
	}
	///@}
};

}


#include "ndarray_view_opaque.tcc"

#endif
