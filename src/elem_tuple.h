#ifndef MF_ELEM_TUPLE_H_
#define MF_ELEM_TUPLE_H_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <type_traits>
#include "elem.h"

namespace mf {

/// Heterogeneous tuple of items.
/** Similar to `std::tuple`, but is also POD type (implies standard layout), and guarantees memory layout. Items
 ** must be `elem` types, but not other `elem_tuple`s.  */
template<typename First_elem, typename... Other_elems>
class elem_tuple {
	static_assert(! elem_traits<First_elem>::is_tuple, "elem_tuple element must not be another tuple");
	
public:
	using others_tuple_type = elem_tuple<Other_elems...>;
	constexpr static bool is_nullable = elem_traits<First_elem>::is_nullable || others_tuple_type::is_nullable;

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
	
	bool is_null() const noexcept { return is_null(first_) || is_null(others_); }
};



template<typename First_elem>
class elem_tuple<First_elem> {
	static_assert(! elem_traits<First_elem>::is_tuple, "elem_tuple element must not be another tuple");

public:
	First_elem first_;
	constexpr static bool is_nullable = elem_traits<First_elem>::is_nullable;

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
	
	bool is_null() const noexcept { return is_null(first_); }
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


/// Index of first element of type `T` in \ref elem_tuple type `Tuple`.
template<typename T, typename Tuple>
constexpr std::ptrdiff_t elem_tuple_index = -1;

template<typename T, typename First_elem, typename... Other_elems>
constexpr std::ptrdiff_t elem_tuple_index<T, elem_tuple<First_elem, Other_elems...>>
	= 1 + elem_tuple_index<T, elem_tuple<Other_elems...>>;
	
template<typename T, typename... Other_elems>
constexpr std::ptrdiff_t elem_tuple_index<T, elem_tuple<T, Other_elems...>>
	= 0;


/// Get element at index `Index` in \ref elem_tuple \a tup.
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


/// Get first element of type `T` in \ref elem_tuple \a tup.
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


/// Offset in bytes of element at index `Index` in \ref elem_tuple type `Tuple`.
template<std::size_t Index, typename Tuple>
constexpr std::ptrdiff_t elem_tuple_offset = -1;

template<std::size_t Index, typename First_elem, typename... Other_elems>
constexpr std::ptrdiff_t elem_tuple_offset<Index, elem_tuple<First_elem, Other_elems...>> =
	detail::elem_tuple_accessor<Index, elem_tuple<First_elem, Other_elems...>>::offset();


/// Make \ref elem_tuple with elements \a elems.
template<typename... Elems>
elem_tuple<Elems...> make_elem_tuple(const Elems&... elems) {
	return elem_tuple<Elems...>(elems...);
}


/// Elem traits specialization of \ref elem_tuple.
template<typename... Elems>
struct elem_traits<elem_tuple<Elems...>> : elem_traits_base<elem_tuple<Elems...>> {
	constexpr static bool is_tuple = true;
	constexpr static bool is_nullable = elem_tuple<Elems...>::is_nullable;
};


}

#endif
