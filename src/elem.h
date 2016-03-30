#ifndef MF_ELEM_H_
#define MF_ELEM_H_

#include <cstddef>
#include <array>
#include <complex>

namespace mf {

namespace detail {
	template<typename Elem>
	struct elem_traits_base {
		static_assert(std::is_pod<Elem>::value, "elem must be POD type");
		
		using scalar_type = Elem;
		constexpr static bool is_tuple = false;
		constexpr static std::size_t components = 1;
		constexpr static std::size_t size = sizeof(Elem);
		constexpr static std::size_t stride = sizeof(Elem);
	};
}


template<typename Elem>
struct elem_traits : detail::elem_traits_base<Elem> { };


template<typename T, std::size_t N>
struct elem_traits<std::array<T, N>> {
	static_assert(std::is_pod<T>::value, "T must be POD type");
	
	using scalar_type = T;
	constexpr static bool is_tuple = false;
	constexpr static std::size_t components = N;
	constexpr static std::size_t size = sizeof(type);
	constexpr static std::size_t stride = sizeof(T);
};


template<typename T, std::size_t N>
struct elem_traits<std::complex<T>> {	
	using scalar_type = T;
	constexpr static bool is_tuple = false;
	constexpr static std::size_t components = 2;
	constexpr static std::size_t size = sizeof(type);
	constexpr static std::size_t stride = sizeof(T);
};


}

#endif
