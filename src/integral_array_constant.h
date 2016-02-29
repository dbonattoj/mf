#ifndef MF_INTEGRAL_ARRAY_CONSTANT_H_
#define MF_INTEGRAL_ARRAY_CONSTANT_H_

#include <array>
#include <utility>

namespace mf {

template<typename T, T... Values>
class integral_array_constant {
public:
	static constexpr std::array<T, sizeof...(Values)> values { {Values...} };
	static constexpr std::size_t size = sizeof...(Values);
	
	constexpr T operator[](std::ptrdiff_t i) const noexcept {
		return values[i];
	}
};


template<std::size_t... Values>
using index_array_constant = integral_array_constant<std::size_t, Values...>;


namespace detail {
	template<std::size_t... Ints>
	constexpr decltype(auto) index_array_constant_from_sequence(std::index_sequence<Ints...> seq) {
		return index_array_constant<Ints...>();
	}
	
	template<std::size_t... Ints>
	constexpr decltype(auto) index_array_constant_from_reverse_sequence(std::index_sequence<Ints...> seq) {
		return index_array_constant<(seq.size() - Ints - 1)...>();
	}
}


template<std::size_t Dim>
using index_array_constant_sequence =
	decltype(detail::index_array_constant_from_sequence(std::make_index_sequence<Dim>()));


template<std::size_t Dim>
using index_array_constant_reverse_sequence =
	decltype(detail::index_array_constant_from_reverse_sequence(std::make_index_sequence<Dim>()));

}

#endif
