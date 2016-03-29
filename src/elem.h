#ifndef MF_ELEM_H_
#define MF_ELEM_H_

#include <cstddef>
#include <cstdint>
#include <utility>

namespace mf {


template<typename First_elem, typename... Other_elems>
class elem_tuple {
private:
	using others_tuple_type = elem_tuple<Other_elems...>;

	First_elem first_;
	others_tuple_type others_;

public:
	elem_tuple() = default;
	elem_tuple(const elem_tuple&) = default;
	elem_tuple(elem_tuple&&) = default;
	
	elem_tuple(First_elem&& first, Other_elems&&... others) :
		first_(std::forward<First_elem>(first)),
		others_(std::forward<Other_elems>(others)...) { }

	elem_tuple& operator=(const elem_tuple&) = default;
	elem_tuple& operator=(elem_tuple&&) = default;
	
	friend bool operator==(const elem_tuple& a, const elem_tuple& b) const {
		return (a.first_ == b.first_) && (a.others_ == b.others_);
	}
	friend bool operator!)=(const elem_tuple& a, const elem_tuple& b) const {
		return (a.first_ != b.first_) || (a.others_ != b.others_);
	}
	
	template<typename T> constexpr static std::ptrdiff_t index() const { return 1 + others_tuple_type::index<T>(); }
	template<> constexpr static std::ptrdiff_t index<First_elem>() const { return 0; }

	
	template<std::ptrdiff_t Index> auto& get() { return others_.get<Index - 1>(); }
	template<> First_elem& get<0>() { return first_; }
	
	template<std::ptrdiff_t Index> const auto& get() const { return others_.get<Index - 1>(); }
	template<> const First_elem& get<0>() const { return first_; }
	

	template<typename T> auto& get() { return get<index<T>()>(); }
	template<typename T> const auto& get() const { return get<index<T>()>(); }
	
	
	template<typename T> constexpr static bool has() { return others_.has<T>(); }
	template<> constexpr static bool has<First_elem>() { return true; }
	
	constexpr static std::size_t size() const { return 1 + sizeof...(Other_elems); }

	template<std::ptrdiff_t Index> constexpr static std::ptrdiff_t offset() const {
		return offsetof(elem_tuple, others_) + others_tuple_type::offset<Index - 1>();
	}
	template<> constexpr static std::ptrdiff_t offset<0>() const {
		return 0;
	}
};


template<typename First_elem>
class elem_tuple<First_elem> {
private:
	First_elem first_;

public:
	elem_tuple() = default;
	elem_tuple(const elem_tuple&) = default;
	elem_tuple(elem_tuple&&) = default;

	elem_tuple(First_elem&& first) :
		first_(std::forward<First_elem>(first)) { }	

	elem_tuple& operator=(const elem_tuple&) = default;
	elem_tuple& operator=(elem_tuple&&) = default;

	friend bool operator==(const elem_tuple& a, const elem_tuple& b) const {
		return (a.first_ == b.first_);
	}
	friend bool operator!=(const elem_tuple& a, const elem_tuple& b) const {
		return (a.first_ != b.first_);
	}

	template<std::ptrdiff_t Index> First_elem& get() {
		static_assert(Index == 0, "index out of range");
		return first_;
	}

	template<std::ptrdiff_t Index> const First_elem& get() const {
		static_assert(Index == 0, "index out of range");
		return first_;
	}
	
	template<typename T> First_elem& get() {
		static_assert(std::is_same_t<T, First_elem>::value, "no such type in tuple");
		return first_;
	}
	template<typename T> const First_elem& get() const {
		static_assert(std::is_same_t<T, First_elem>::value, "no such type in tuple");
		return first_;
	}

	template<typename T>
	constexpr static std::ptrdiff_t index() const {
		static_assert(std::is_same_t<T, First_elem>::value, "no such type in tuple");
		return 0;
	}
	
	template<typename T> constexpr static bool has() {
		return std::is_same_t<T, First_elem>::value;
	}
	
	constexpr static std::size_t size() const { return 1; }
	
	template<std::ptrdiff_t Index> constexpr std::ptrdiff_t offset() const {
		static_assert(Index == 0, "index out of range");
		return 0;
	}
};

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


template<typename Elem>
struct elem_traits {
	static_assert(std::is_scalar<Elem>::value, "no elem_traits specialization and type is not scalar");

	using scalar_type = Elem;
	using type = Elem;
	enum {
		dimension = 1,
		size = sizeof(Elem),
		align = alignof(Elem)
	}
};


}

#endif