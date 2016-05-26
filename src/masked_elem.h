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

#ifndef MF_MASKED_ELEM_H_
#define MF_MASKED_ELEM_H_

#include "elem.h"
#include "common.h"
#include <cstddef>
#include <type_traits>
#include <climits>

namespace mf {
	
using masked_elem_flag_index = unsigned;

/// Nullable wrapper for elem type which adds mask.
/** POD type, where the elem is always at same memory address as the `masked_elem` object. A one byte mask is added
 ** behind the element. Depending in alignmment requirement of `Elem`, `sizeof(masked_elem<Elem>)` can become larger
 ** than `sizeof(Elem) + 1`. 
 ** `Elem` must be non-nullable. `masked_elem::elem` is the contained element. `masked_elem::mask` is the one byte
 ** mask. The `masked_elem<Elem>` is null iff `mask == 0`. Otherwise, `mask` can contain an arbitrary non-zero value.
 ** Bitwise flags can be set using get_flag(), set_flag() and unset_flag().
 ** With \ref ndarray_view_cast, an \ref ndarray_view of \ref masked_elem elements can be casted to the unmasked
 ** `ndarray_view<Dim, Elem>`, or to the mask with `ndarray_view<Dim, bool>`. */ 
template<typename Elem, typename = void>
struct masked_elem {	
	static_assert(! std::is_const<Elem>::value, "masked_elem cannot have const Elem");
	static_assert(! elem_traits<Elem>::is_nullable, "masked_elem elem cannot be nullable in this specialization");
	
	Elem elem; ///< Non-nullable element.
	byte mask; ///< Mask, false if masked_elem is null.
	
	static masked_elem null() { return masked_elem(); }
	bool is_null() const { return ! mask; }
	explicit operator bool () const { return ! is_null(); }
	
	static byte flag_mask_bitmask(masked_elem_flag_index i) { MF_ASSERT(i < CHAR_BIT); return (1 << i); }
	bool get_flag(masked_elem_flag_index i) const { return mask & flag_mask_bitmask(i); }
	void set_flag(masked_elem_flag_index i) { mask |= flag_mask_bitmask(i); }
	void unset_flag(masked_elem_flag_index i) { mask &= ~flag_mask_bitmask(i); }
	
	masked_elem() : elem(), mask(false) { }
	masked_elem(const Elem& el) : elem(el), mask(true) { }
	masked_elem(const masked_elem&) = default;
	
	masked_elem& operator=(const masked_elem&) = default;
	masked_elem& operator=(const Elem& el) { elem = el; mask = true; return *this; }
	
	friend bool operator==(const masked_elem& a, const masked_elem& b) {
		if(a.mask && b.mask) return true;
		else if(a.mask != b.mask) return false;
		else return (a.elem == b.elem);
	}
	friend bool operator!=(const masked_elem& a, const masked_elem& b) {
		return !(a == b);
	}
	
	operator const Elem& () const { MF_EXPECTS(mask); return elem; }
	operator Elem& () { MF_EXPECTS(mask); return elem; }
};


// TODO FIX ndarray_cast<bool> where mask is not bool type

// TODO nullptr_t-like object
// TODO const support
// TODO disallow masked_elem<bool> (ndarray_view_cast...)
// TODO data hiding, allow overwrite elem if masked => allow flags also if masked


/// Elem traits specialization for masked elem.
/** Always nullable. */
template<typename Elem>
struct elem_traits<masked_elem<Elem>> :
	elem_traits_base<masked_elem<Elem>, masked_elem<Elem>, 1, true> { };


namespace detail {
	template<typename Elem>
	struct unmasked_type {
		using type = Elem;
	};
	
	template<typename Elem>
	struct unmasked_type<masked_elem<Elem>> {
		using type = Elem;
	};
	
	template<typename Elem>
	struct masked_type {
		using type = masked_elem<Elem>;
	};
	
	template<typename Elem>
	struct masked_type<masked_elem<Elem>> {
		using type = masked_elem<Elem>;
	};
}


/// Add mask to type `Elem`, if not already a `masked_type`.
template<typename Elem> using masked_type = typename detail::masked_type<Elem>::type;


/// Remove mask to type `Elem`, if is is a `masked_type`.
template<typename Elem> using unmasked_type = typename detail::unmasked_type<Elem>::type;


}

#endif
