#ifndef MF_MASKED_ELEM_H_
#define MF_MASKED_ELEM_H_

#include "elem.h"
#include <cstddef>
#include <type_traits>

namespace mf {

/// Nullable wrapper for elem type wich adds mask.
/** POD type, where the elem is always at same memory address as the `masked_elem` object. A one byte mask is added
 ** behind the element. Depending in alignmment requirement of `Elem`, `sizeof(masked_elem<Elem>)` can become larger
 ** than `sizeof(Elem) + 1`. */ 
template<typename Elem>
struct masked_elem {
	static_assert(! std::is_const<Elem>::value, "masked_elem cannot have const Elem");
	
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
	
	operator const Elem& () const { MF_EXPECTS(mask); return elem; }
	operator Elem& () { MF_EXPECTS(mask); return elem; }
};

// TODO specialization for nullable Elem (null = masked)
// TODO const support

template<typename Elem>
struct elem_traits<masked_elem<Elem>> :
	elem_traits_base<masked_elem<Elem>, masked_elem<Elem>, 1, true> { };

}

#endif
