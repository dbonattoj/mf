#ifndef MF_NDARRAY_VIEW_H_
#define MF_NDARRAY_VIEW_H_

#include "ndcoord.h"
#include "ndarray_iterator.h"
#include "util.h"
#include <type_traits>
#include <utility>
#include <memory>

namespace mf {

template<std::size_t Dim, typename T> class ndarray_view;

namespace detail {
	template<std::size_t Dim, typename T>
	ndarray_view<Dim - 1, T> get_subscript(const ndarray_view<Dim, T>& array, std::ptrdiff_t c) {
		return array.slice(c, 0);
	}
	
	template<typename T>
	T& get_subscript(const ndarray_view<1, T>& array, std::ptrdiff_t c) {
		return array.at({c});
	}
	
	template<std::size_t Dim, typename T, std::ptrdiff_t I>
	class ndarray_view_fcall;
}

template<std::size_t Dim, typename T>
class ndarray_view {
	static_assert(Dim >= 1, "ndarray_view dimension must be >= 1");

public:
	using value_type = T;
	using pointer = T*;
	using reference = T&;
	using index_type = std::ptrdiff_t;
	using coordinates_type = ndptrdiff<Dim>;
	using shape_type = ndsize<Dim>;
	using strides_type = ndptrdiff<Dim>;
	
	using iterator = ndarray_iterator<ndarray_view>;

	constexpr static std::size_t dimension = Dim;

protected:
	pointer start_;
	shape_type shape_;
	strides_type strides_;
	
	std::ptrdiff_t contiguous_length_;
	
	ndarray_view section_(std::ptrdiff_t dimension, std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const;
	std::ptrdiff_t fix_coordinate_(std::ptrdiff_t c, std::ptrdiff_t dimension) const;
			
public:
	static strides_type default_strides(const shape_type&);

	ndarray_view(pointer start, const shape_type& shape, const strides_type& strides);
	ndarray_view(pointer start, const shape_type& shape);
	
	ndarray_view(const ndarray_view<Dim, std::remove_const_t<T>>& arr) :
		ndarray_view(arr.start(), arr.shape(), arr.strides()) { }
	
	ndarray_view& operator=(const ndarray_view&) = default;
	
	coordinates_type index_to_coordinates(const index_type&) const;
	index_type coordinates_to_index(const coordinates_type&) const;
	pointer coordinates_to_pointer(const coordinates_type&) const;
		
	ndarray_view section(const coordinates_type& start, const coordinates_type& end, const strides_type& steps = strides_type(1)) const;
	ndarray_view<Dim - 1, T> slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const;
	
	auto operator[](std::ptrdiff_t c) const {
		return detail::get_subscript(*this, c);
	}
	
	detail::ndarray_view_fcall<Dim, T, 1> operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const {
		return section_(0, start, end, step);
	}
	detail::ndarray_view_fcall<Dim, T, 1> operator()(std::ptrdiff_t c) const {
		return section_(0, c, c + 1, 1);
	}
	detail::ndarray_view_fcall<Dim, T, 1> operator()() const {
		return *this;
	}
	
	reference at(const coordinates_type&) const;
			
	iterator begin() const;
	iterator end() const;
	
	friend bool operator==(const ndarray_view& a, const ndarray_view& b) noexcept {
		return (a.start_ == b.start_) && (a.shape_ == b.shape_) && (a.strides_ == b.strides_);
	}
	friend bool operator!=(const ndarray_view& a, const ndarray_view& b) noexcept {
		return (a.start_ != b.start_) || (a.shape_ != b.shape_) || (a.strides_ != b.strides_);
	}
	
	std::size_t size() const { return shape().product(); }
	
	pointer start() const noexcept { return start_; }
	const shape_type& shape() const noexcept { return shape_; }
	const strides_type& strides() const noexcept { return strides_; }
	std::ptrdiff_t contiguous_length() const noexcept { return contiguous_length_; }
};


template<typename Base, typename Array>
inline auto ndarray_view_cast(const Array& arr) {
	using derived_type = typename Array::value_type;
	using base_type = Base;
	
	static_assert(std::is_base_of<Base, derived_type>::value, "Base must be base of Array::value_type");
	if(reinterpret_cast<base_type*>(arr.start()) != static_cast<base_type*>(arr.start()))
		throw std::invalid_argument("address of derived type must be same as address of base type");
	return ndarray_view<Array::dimension, base_type>(
		reinterpret_cast<Base*>(arr.start()),
		arr.shape(),
		arr.strides()
	);
}

}

#include "ndarray_view.tcc"

#endif