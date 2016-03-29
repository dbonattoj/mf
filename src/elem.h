#ifndef MF_ELEM_H_
#define MF_ELEM_H_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <type_traits>
#include <array>

namespace mf {

namespace detail {
	template<typename Elem>
	struct elem_traits_base {
		static_assert(std::is_pod<Elem>::value, "elem must be POD type");
		
		using scalar_type = Elem;
		using type = Elem;
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
	using type = std::array<T, N>;
	constexpr static bool is_tuple = false;
	constexpr static std::size_t components = N;
	constexpr static std::size_t size = sizeof(type);
	constexpr static std::size_t stride = sizeof(T);
};


template<typename First_elem, typename... Other_elems>
class elem_tuple {
	static_assert(! elem_traits<First_elem>::is_tuple, "elem_tuple element must not be another tuple");
	
public:
	using others_tuple_type = elem_tuple<Other_elems...>;

	First_elem first_;
	others_tuple_type others_;

public:
	elem_tuple() = default;
	elem_tuple(const elem_tuple&) = default;
	elem_tuple(elem_tuple&&) = default;
	
	elem_tuple(const First_elem& first, const Other_elems&... others) :
		first_(first), others_(others...) { }

	elem_tuple& operator=(const elem_tuple&) = default;
	elem_tuple& operator=(elem_tuple&&) = default;
	
	friend bool operator==(const elem_tuple& a, const elem_tuple& b) {
		return (a.first_ == b.first_) && (a.others_ == b.others_);
	}
	friend bool operator!=(const elem_tuple& a, const elem_tuple& b) {
		return (a.first_ != b.first_) || (a.others_ != b.others_);
	}
	
	constexpr static std::size_t size() { return 1 + sizeof...(Other_elems); }
};



template<typename First_elem>
class elem_tuple<First_elem> {
	static_assert(! elem_traits<First_elem>::is_tuple, "elem_tuple element must not be another tuple");

public:
	First_elem first_;

public:
	elem_tuple() = default;
	elem_tuple(const elem_tuple&) = default;
	elem_tuple(elem_tuple&&) = default;

	explicit elem_tuple(const First_elem& first) :
		first_(first) { }	

	elem_tuple& operator=(const elem_tuple&) = default;
	elem_tuple& operator=(elem_tuple&&) = default;

	friend bool operator==(const elem_tuple& a, const elem_tuple& b) {
		return (a.first_ == b.first_);
	}
	friend bool operator!=(const elem_tuple& a, const elem_tuple& b) {
		return (a.first_ != b.first_);
	}
	
	constexpr static std::size_t size() { return 1; }
};



namespace detail {

	template<std::ptrdiff_t Index, typename Tuple>
	struct elem_tuple_accessor;
	
	
	template<std::ptrdiff_t Index, typename First_elem, typename... Other_elems>
	struct elem_tuple_accessor<Index, elem_tuple<First_elem, Other_elems...>> {
		using tuple_type = elem_tuple<First_elem, Other_elems...>;
		using others_tuple_type = typename tuple_type::others_tuple_type;
		
		static auto& get(tuple_type& tup) {
			return elem_tuple_accessor<Index - 1, others_tuple_type>::get(tup.others_);
		}
		
		static const auto& get(const tuple_type& tup) {
			return elem_tuple_accessor<Index - 1, others_tuple_type>::get(tup.others_);
		}
		
		constexpr static std::ptrdiff_t offset() {
			return offsetof(tuple_type, others_) + elem_tuple_accessor<Index - 1, others_tuple_type>::offset();
		}
	};
	
	
	template<typename First_elem, typename... Other_elems>
	struct elem_tuple_accessor<0, elem_tuple<First_elem, Other_elems...>> {
		using tuple_type = elem_tuple<First_elem, Other_elems...>;
		
		static auto& get(tuple_type& tup) {
			return tup.first_;
		}
		static const auto& get(const tuple_type& tup) {
			return tup.first_;
		}
		constexpr static std::ptrdiff_t offset() {
			return 0;
		}
	};

}


template<typename T, typename Tuple>
constexpr std::ptrdiff_t elem_tuple_index = -1;

template<typename T, typename First_elem, typename... Other_elems>
constexpr std::ptrdiff_t elem_tuple_index<T, elem_tuple<First_elem, Other_elems...>>
	= 1 + elem_tuple_index<T, elem_tuple<Other_elems...>>;
	
template<typename T, typename... Other_elems>
constexpr std::ptrdiff_t elem_tuple_index<T, elem_tuple<T, Other_elems...>>
	= 0;


template<std::size_t Index, typename First_elem, typename... Other_elems>
const auto& get(const elem_tuple<First_elem, Other_elems...>& tup) {
	using tuple_type = std::decay_t<decltype(tup)>;
	return detail::elem_tuple_accessor<Index, tuple_type>::get(tup);
}

template<std::size_t Index, typename First_elem, typename... Other_elems>
auto& get(elem_tuple<First_elem, Other_elems...>& tup) {
	using tuple_type = std::decay_t<decltype(tup)>;
	return detail::elem_tuple_accessor<Index, tuple_type>::get(tup);
}


template<typename T, typename First_elem, typename... Other_elems>
const auto& get(const elem_tuple<First_elem, Other_elems...>& tup) {
	using tuple_type = std::decay_t<decltype(tup)>;
	constexpr std::size_t index = elem_tuple_index<T, tuple_type>;
	return detail::elem_tuple_accessor<index, tuple_type>::get(tup);
}

template<typename T, typename First_elem, typename... Other_elems>
auto& get(elem_tuple<First_elem, Other_elems...>& tup) {
	using tuple_type = std::decay_t<decltype(tup)>;
	constexpr std::size_t index = elem_tuple_index<T, tuple_type>;
	return detail::elem_tuple_accessor<index, tuple_type>::get(tup);
}


template<std::size_t Index, typename Tuple>
constexpr std::ptrdiff_t elem_tuple_offset = -1;

template<std::size_t Index, typename First_elem, typename... Other_elems>
constexpr std::ptrdiff_t elem_tuple_offset<Index, elem_tuple<First_elem, Other_elems...>> =
	detail::elem_tuple_accessor<Index, elem_tuple<First_elem, Other_elems...>>::offset();




template<typename... Elems>
struct elem_traits<elem_tuple<Elems...>> : detail::elem_traits_base<elem_tuple<Elems...>> {
	constexpr static bool is_tuple = true;
};


}

#endif



/*
namespace detail {
	template<
		typename... Left_elems, typename... Right_elems...,
		std::size_t... Left_is, std::size_t... Right_is>
	auto elem_tuple_cat(
		const elem_tuple<Left_elems...>& left,
		const elem_tuple<Right_elems...>& right,
		std::index_sequence<Left_is...> = std::make_index_sequence<sizeof...(Left_elems)>
		std::index_sequence<Right_is...> = std::make_index_sequence<sizeof...(Right_elems)>
	) {
		using result_type = elem_tuple<Left_elems..., Right_elems...>;
		return result_type(
			left.get(left_is)...,
			right.get(right_is)...
		);
	}
}
*/

