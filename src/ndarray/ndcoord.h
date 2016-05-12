#ifndef MF_NDCOORD_H_
#define MF_NDCOORD_H_

#include <cstddef>
#include <cassert>
#include <array>
#include <initializer_list>
#include <functional>
#include <ostream>
#include <type_traits>

namespace mf {

/// Vector of n-dimensional coordinates.
/** Vector of size `Dim` with items of arithmetic type `T`. Used mainly for coordinates of `ndarray_view`.
 ** For real-values vectors expressing positions or directions in space, the `Eigen` vector types are used instead. */
template<std::size_t Dim, typename T>
class ndcoord {
	static_assert(std::is_arithmetic<T>::value, "ndcoord component type must be arithmetic");

private:
	std::array<T, Dim> components_;

public:
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using iterator = typename std::array<T, Dim>::iterator;
	using const_iterator = typename std::array<T, Dim>::const_iterator;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	
	ndcoord(T value = 0) noexcept { components_.fill(value); }
	
	template<typename It>
	ndcoord(It begin, It end) {
		auto out = components_.begin();
		for(auto in = begin; in != end; ++in, ++out)
			*out = static_cast<T>(*in);
	}
		
	ndcoord(std::initializer_list<T> l) noexcept :
		ndcoord(l.begin(), l.end()) { }
			
	ndcoord(const ndcoord&) = default;
	
	template<typename T2>
	ndcoord(const ndcoord<Dim, T2>& coord) :
		ndcoord(coord.begin(), coord.end()) { }
	
	T& operator[](std::ptrdiff_t i) noexcept {
		assert(i >= 0 && i < Dim);
		return components_[i];
	}
	const T& operator[](std::ptrdiff_t i) const noexcept {
		assert(i >= 0 && i < Dim);
		return components_[i];
	}
	
	iterator begin() noexcept { return components_.begin(); }
	const_iterator begin() const noexcept { return components_.cbegin(); }
	const_iterator cbegin() const noexcept { return components_.cbegin(); }
	iterator end() noexcept { return components_.end(); }
	const_iterator end() const noexcept { return components_.cend(); }
	const_iterator cend() const noexcept { return components_.cend(); }
	size_type size() const noexcept { return Dim; }
	
	template<typename Unary>
	ndcoord& transform_inplace(Unary fct) {
		for(T& c : *this) c = fct(c);
		return *this;
	}
	
	template<typename Binary>
	ndcoord& transform_inplace(const ndcoord& c, Binary fct) {
		for(std::ptrdiff_t i = 0; i < Dim; ++i)
			components_[i] = fct(components_[i], c[i]);
		return *this;
	}
	
	ndcoord& operator+=(const ndcoord& c) noexcept { return transform_inplace(c, std::plus<T>()); }
	ndcoord& operator-=(const ndcoord& c) noexcept { return transform_inplace(c, std::minus<T>()); }
	ndcoord& operator*=(const ndcoord& c) noexcept { return transform_inplace(c, std::multiplies<T>()); }
	ndcoord& operator/=(const ndcoord& c) noexcept { return transform_inplace(c, std::divides<T>()); }

	ndcoord& operator*=(T val) noexcept { return operator*=(ndcoord(val)); };
	ndcoord& operator/=(T val) noexcept { return operator/=(ndcoord(val)); };

	ndcoord operator+() noexcept { return *this; }
	ndcoord operator-() noexcept { return transform_inplace(std::negate<T>()); }

	friend ndcoord operator+(const ndcoord& a, const ndcoord& b) noexcept
		{ return transform(a, b, std::plus<T>()); }
	friend ndcoord operator-(const ndcoord& a, const ndcoord& b) noexcept
		{ return transform(a, b, std::minus<T>()); }
	friend ndcoord operator*(const ndcoord& a, const ndcoord& b) noexcept
		{ return transform(a, b, std::multiplies<T>()); }
	friend ndcoord operator/(const ndcoord& a, const ndcoord& b) noexcept
		{ return transform(a, b, std::divides<T>()); }

	friend ndcoord operator*(const ndcoord& a, T val) noexcept
		{ return a * ndcoord(val); }
	friend ndcoord operator/(const ndcoord& a, T val) noexcept
		{ return a / ndcoord(val); }
		
	friend bool operator==(const ndcoord& a, const ndcoord& b) noexcept
		{ return a.components_ == b.components_; }
	friend bool operator!=(const ndcoord& a, const ndcoord& b) noexcept
		{ return a.components_ != b.components_; }
	
	T product() const noexcept {
		T prod = 1;
		for(T c : components_) prod *= c;
		return prod;
	}
	
	const T& front() const noexcept { return components_.front(); }
	T& front() noexcept { return components_.front(); }
	const T& back() const noexcept { return components_.back(); }
	T& back() noexcept { return components_.back(); }	
	
	template<std::size_t Section_dim = Dim - 1>
	auto tail() const noexcept {
		ndcoord<Section_dim, T> c(begin() + (Dim - Section_dim), end());
		return c;
	}

	template<std::size_t Section_dim = Dim - 1>
	auto head() const noexcept {
		ndcoord<Section_dim, T> c(begin(), end() - (Dim - Section_dim));
		return c;
	}
		
	ndcoord<Dim - 1, T> erase(std::ptrdiff_t i) const noexcept {
		ndcoord<Dim - 1, T> result;
		for(std::ptrdiff_t j = 0; j < i; ++j) result[j] = components_[j];
		for(std::ptrdiff_t j = i + 1; j < Dim; ++j) result[j - 1] = components_[j];
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
std::ostream& operator<<(std::ostream& str, const ndcoord<Dim, T>& coord) {
	str << '(';
	for(std::ptrdiff_t i = 0; i < Dim - 1; i++) str << coord[i] << ", ";
	str << coord.back() << ')';
	return str;
}


template<typename T>
std::ostream& operator<<(std::ostream& str, const ndcoord<0, T>& coord) {
	return str << "()";
}


template<std::size_t Dim>
using ndsize = ndcoord<Dim, std::size_t>;

template<std::size_t Dim>
using ndptrdiff = ndcoord<Dim, std::ptrdiff_t>;


template<typename T, typename... Components>
auto make_ndcoord(Components... c) {
	return ndcoord<sizeof...(Components), T>({ static_cast<T>(c)... });
}

template<typename... Components>
auto make_ndsize(Components... c) {
	return make_ndcoord<std::size_t>(c...);
}

template<typename... Components>
auto make_ndptrdiff(Components... c) {
	return make_ndcoord<std::ptrdiff_t>(c...);
}


template<std::size_t Dim1, std::size_t Dim2, typename T>
ndcoord<Dim1 + Dim2, T> ndcoord_cat(const ndcoord<Dim1, T>& coord1, const ndcoord<Dim2, T>& coord2) {
	ndcoord<Dim1 + Dim2, T> coord;
	if(Dim1 + Dim2 == 0) return coord;
	auto it = coord.begin();
	if(Dim1 > 0) for(T c : coord1) *(it++) = c;
	if(Dim2 > 0) for(T c : coord2) *(it++) = c;
	return coord;
}


template<std::size_t Dim1, typename T, typename Int>
ndcoord<Dim1 + 1, T> ndcoord_cat(const ndcoord<Dim1, T>& coord1, Int c2) {
	return ndcoord_cat(coord1, make_ndcoord<T>(c2));
}


template<std::size_t Dim2, typename T, typename Int>
ndcoord<1 + Dim2, T> ndcoord_cat(Int c1, const ndcoord<Dim2, T>& coord2) {
	return ndcoord_cat(make_ndcoord<T>(c1), coord2);
}


template<typename T>
ndcoord<2, T> flip(const ndcoord<2, T>& coord) {
	return {coord[1], coord[0]};
}


}

#endif
