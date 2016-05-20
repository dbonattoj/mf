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

#ifndef MF_ELEM_H_
#define MF_ELEM_H_

#include <cstddef>
#include <array>
#include <complex>
#include <type_traits>
#include "common.h"

namespace mf {
	
/// Elem traits base class with the required members.
template<typename Elem, typename Scalar = Elem, std::size_t Components = 1, bool Nullable = false>
struct elem_traits_base {
	static_assert(std::is_standard_layout<Elem>::value, "elem must be standard layout type");
	static_assert(std::is_standard_layout<Scalar>::value, "elem scalar must be standard layout type");
	
	using scalar_type = Scalar;
	constexpr static bool is_tuple = false;
	constexpr static std::size_t components = Components;
	constexpr static std::size_t stride = sizeof(Scalar);
	
	constexpr static bool is_nullable = Nullable;
};


/// Default elem traits, using Elem as standard layout scalar type.
/** `Elem` must be standard layout type. */
template<typename Elem>
struct elem_traits : elem_traits_base<Elem> { };


/// Elem traits specialization for `std::array<T, N>`.
/** `T` must be standard layout type. */
template<typename T, std::size_t N>
struct elem_traits<std::array<T, N>> :
	elem_traits_base<std::array<T, N>, T, N> { };

/// Elem traits specialization for `std::complex<T>`.
template<typename T>
struct elem_traits<std::complex<T>> :
	elem_traits_base<std::complex<T>, T, 2> { };


template<typename Elem>
std::enable_if_t<elem_traits<Elem>::is_nullable, bool> is_null(const Elem& elem) {
	return elem.is_null();
}


template<typename Elem>
std::enable_if_t<! elem_traits<Elem>::is_nullable, bool> is_null(const Elem& elem) {
	return false;
}


}

#endif
