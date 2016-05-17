#ifndef MF_MASKED_ELEM_H_
#define MF_MASKED_ELEM_H_

#include "elem.h"
#include <cstddef>
#include <type_traits>

namespace mf {

/// Nullable wrapper for elem type which adds mask.
/** POD type, where the elem is always at same memory address as the `masked_elem` object. A one byte mask is added
 ** behind the element. Depending in alignmment requirement of `Elem`, `sizeof(masked_elem<Elem>)` can become larger
 ** than `sizeof(Elem) + 1`. */ 
template<typename Elem, typename = void>
struct masked_elem {
	static_assert(! std::is_const<Elem>::value, "masked_elem cannot have const Elem");
	static_assert(! elem_traits<Elem>::is_nullable, "masked_elem elem cannot be nullable in this specialization");
	
	Elem elem; ///< Non-nullable element.
	bool mask; ///< Mask, false if masked_elem is null.
	
	static masked_elem null() { return masked_elem(); }
	bool is_null() const { return ! mask; }
	explicit operator bool () const { return ! is_null(); }
	
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


/// Masked elem specialization where `Elem` is already nullable.
/** Doesn't add separate mask. */
template<typename Elem>
class masked_elem<Elem, std::enable_if_t<elem_traits<Elem>::is_nullable>> {
	static_assert(! std::is_const<Elem>::value, "masked_elem cannot have const Elem");
	static_assert(elem_traits<Elem>::is_nullable, "masked_elem elem must be nullable in this specialization");

	Elem elem; ///< Nullable element.
	
	static masked_elem null() { return masked_elem(); }
	bool is_null() const { return elem.is_null(); }
	explicit operator bool () const { return ! is_null(); }
	
	masked_elem() : elem(Elem::null()) { }
	masked_elem(const Elem& el) : elem(el) { }
	masked_elem(const masked_elem&) = default;
	
	masked_elem& operator=(const masked_elem&) = default;
	masked_elem& operator=(const Elem& el) { elem = el; return *this; }
	
	friend bool operator==(const masked_elem& a, const masked_elem& b) {
		return (a.elem == b.elem);
	}
	friend bool operator!=(const masked_elem& a, const masked_elem& b) {
		return !(a == b);
	}
	
	operator const Elem& () const { return elem; }
	operator Elem& () { return elem; }
};


// TODO const support

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
