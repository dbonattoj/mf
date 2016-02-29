#ifndef MF_NDCOORD_H_
#define MF_NDCOORD_H_

#include <cstddef>
#include <cassert>
#include <array>
#include <initializer_list>
#include <functional>
#include <ostream>

namespace mf {

template<std::size_t Dim, typename T = std::ptrdiff_t>
struct ndcoord {
	std::array<T, Dim> components;
	
	template<typename It>
	void from_iterators(It begin, It end) {
		auto out = components.begin();
		for(auto in = begin; in != end; ++in, ++out)
			*out = static_cast<T>(*in);
	}

	ndcoord() noexcept { components.fill(0); }
	explicit ndcoord(T value) noexcept { components.fill(value); }
		
	template<typename... Cs>
	ndcoord(T first, T second, Cs... others) noexcept :
	components{ {static_cast<T>(first), static_cast<T>(second), static_cast<T>(others)...} } {
		static_assert(Dim >= 2 && sizeof...(Cs) == Dim - 2, "number of coordinates must equal Dim");
	}
		
	ndcoord(std::initializer_list<T> l) noexcept {
		from_iterators(l.begin(), l.end());
	}
			
	ndcoord(const ndcoord&) = default;
	
	template<typename T2>
	ndcoord(const ndcoord<Dim, T2>& coord) {
		from_iterators(coord.components.begin(), coord.components.end());
	}
	
	T& operator[](std::ptrdiff_t i) noexcept {
		assert(i >= 0 && i < Dim);
		return components[i];
	}
	const T& operator[](std::ptrdiff_t i) const noexcept {
		assert(i >= 0 && i < Dim);
		return components[i];
	}
	
	template<typename Unary>
	ndcoord& transform_inplace(Unary fct) {
		for(T& component : components) component = fct(component);
		return *this;
	}
	
	template<typename Binary>
	ndcoord& transform_inplace(const ndcoord& c, Binary fct) {
		for(std::ptrdiff_t i = 0; i < Dim; ++i)
			components[i] = fct(components[i], c.components[i]);
		return *this;
	}
	
	ndcoord& operator+=(const ndcoord& c) noexcept { return transform_inplace(c, std::plus<T>()); }
	ndcoord& operator-=(const ndcoord& c) noexcept { return transform_inplace(c, std::minus<T>()); }
	ndcoord& operator*=(const ndcoord& c) noexcept { return transform_inplace(c, std::multiplies<T>()); }
	ndcoord& operator/=(const ndcoord& c) noexcept { return transform_inplace(c, std::divides<T>()); }

	ndcoord operator+() noexcept { return *this; }
	ndcoord operator-() noexcept { return transform_inplace(std::negate<T>()); }

	friend ndcoord operator+(const ndcoord& a, const ndcoord& b) noexcept { return transform(a, b, std::plus<T>()); }
	friend ndcoord operator-(const ndcoord& a, const ndcoord& b) noexcept { return transform(a, b, std::minus<T>()); }
	friend ndcoord operator*(const ndcoord& a, const ndcoord& b) noexcept { return transform(a, b, std::multiplies<T>()); }
	friend ndcoord operator/(const ndcoord& a, const ndcoord& b) noexcept { return transform(a, b, std::divides<T>()); }
	
	
	T product() const noexcept {
		T prod = 1;
		for(T component : components) prod *= component;
		return prod;
	}
	
	const T& front() const noexcept { return components.front(); }
	T& front() noexcept { return components.front(); }
	const T& back() const noexcept { return components.back(); }
	T& back() noexcept { return components.back(); }	
	
	ndcoord<Dim - 1, T> tail() const noexcept {
		ndcoord<Dim - 1, T> c;
		c.from_iterators(components.begin() + 1, components.end());
		return c;
	}
			
	ndcoord<Dim - 1, T> head() const noexcept {
		ndcoord<Dim - 1, T> c;
		c.from_iterators(components.begin(), components.end() - 1);
		return c;
	}
	
	ndcoord<Dim - 1, T> erase(std::ptrdiff_t i) const noexcept {
		ndcoord<Dim - 1, T> result;
		for(std::ptrdiff_t j = 0; j < i; ++j) result.components[j] = components[j];
		for(std::ptrdiff_t j = i + 1; j < Dim; ++j) result.components[j - 1] = components[j];
		return result;
	}
};


template<std::size_t Dim, typename T, typename Unary>
ndcoord<Dim, T> transform(const ndcoord<Dim, T>& a, Unary fct) {
	ndcoord<Dim, T> o;
	for(std::ptrdiff_t i = 0; i < Dim; ++i) o[i] = fct(a[i]);
	return o;
}


template<std::size_t Dim, typename T, typename Binary>
ndcoord<Dim, T> transform(const ndcoord<Dim, T>& a, const ndcoord<Dim, T>& b, Binary fct) {
	ndcoord<Dim, T> o;
	for(std::ptrdiff_t i = 0; i < Dim; ++i) o[i] = fct(a[i], b[i]);
	return o;
}



template<std::size_t Dim, typename T>
bool operator==(const ndcoord<Dim, T>& a, const ndcoord<Dim, T>& b) noexcept {
	return a.components == b.components;
}


template<std::size_t Dim, typename T>
bool operator!=(const ndcoord<Dim, T>& a, const ndcoord<Dim, T>& b) noexcept {
	return a.components != b.components;
}


template<std::size_t Dim, typename T>
std::ostream& operator<<(std::ostream& str, const ndcoord<Dim, T>& coord) {
	str << '(';
	for(std::ptrdiff_t i = 0; i < Dim - 1; i++) str << coord.components[i] << ", ";
	str << coord.components.back() << ')';
	return str;
}


template<std::size_t Dim>
using ndsize = ndcoord<Dim, std::size_t>;

template<std::size_t Dim>
using ndptrdiff = ndcoord<Dim, std::ptrdiff_t>;

}

#endif