#ifndef MF_ELEM_H_
#define MF_ELEM_H_

#include <cstddef>
#include <array>
#include <complex>

namespace mf {

template<typename Elem, typename Scalar = Elem, std::size_t Components = 1>
struct elem_traits_base {
	static_assert(std::is_standard_layout<Elem>::value, "elem must be standard layout type");
	static_assert(std::is_standard_layout<Scalar>::value, "elem scalar must be standard layout type");
	
	using scalar_type = Scalar;
	constexpr static bool is_tuple = false;
	constexpr static std::size_t components = Components;
	constexpr static std::size_t stride = sizeof(Scalar);
	constexpr static std::size_t offset = 0;
};


template<typename Elem>
struct elem_traits : elem_traits_base<Elem> { };

template<typename T, std::size_t N>
struct elem_traits<std::array<T, N>> :
	elem_traits_base<std::array<T, N>, T, N> { };


template<typename T>
struct elem_traits<std::complex<T>> :
	elem_traits_base<std::complex<T>, T, 2> { };


}

#endif
