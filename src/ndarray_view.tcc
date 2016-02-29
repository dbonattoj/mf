#include <stdexcept>
#include <cstdlib>
#include <iostream>

namespace mf {

namespace detail {
	template<std::size_t Dim, typename T, std::ptrdiff_t I>
	class ndarray_view_fcall : public ndarray_view<Dim, T> {
		static_assert(I <= Dim, "detail::ndarray_view_fcall target dimension out of bounds");
	
	public:
		using super = ndarray_view<Dim, T>;

		ndarray_view_fcall(const super& arr) :
			super(arr) { }
		
		ndarray_view_fcall<Dim, T, I + 1> operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const {
			return this->section_(I, start, end, step);
		}
		ndarray_view_fcall<Dim, T, I + 1> operator()(std::ptrdiff_t c) const {
			return this->section_(I, c, c + 1, 1);
		}
		ndarray_view_fcall<Dim, T, I + 1> operator()() const {
			return *this;
		}
	};
}


template<std::size_t Dim, typename T>
constexpr std::size_t ndarray_view<Dim, T>::dimension;


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::row_major_strides(const shape_type& shape) -> strides_type {
	strides_type strides;
	strides[Dim - 1] = sizeof(T);
	for(std::ptrdiff_t i = Dim - 1; i > 0; --i)
		strides[i - 1] = strides[i] * shape[i];
	return strides;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::column_major_strides(const shape_type& shape) -> strides_type {
	strides_type strides;
	strides[0] = sizeof(T);
	for(std::ptrdiff_t i = 0; i < Dim - 1; ++i)
		strides[i + 1] = strides[i] * shape[i];
	return strides;
}


template<std::size_t Dim, typename T>
ndarray_view<Dim, T>::ndarray_view(pointer start, const shape_type& shape, ndarray_order order) :
	ndarray_view(
		start,
		shape,
		order,
		(order == ndarray_order::row_major) ? row_major_strides(shape) : column_major_strides(shape)
	) { }



template<std::size_t Dim, typename T>
ndarray_view<Dim, T>::ndarray_view(pointer start, const shape_type& shape, ndarray_order order, const strides_type& strides) :
	start_(start), shape_(shape), strides_(strides), order_(order)
{
	std::ptrdiff_t i;
	
	if(order_ == ndarray_order::row_major) {
		contiguous_length_ = shape_.back();	
		for(i = Dim - 1; i > 0; i--) {
			if(strides_[i - 1] == shape_[i] * strides_[i]) contiguous_length_ *= shape_[i - 1];
			else break;
		}
	
	} else {
		contiguous_length_ = shape_.front();	
		for(i = 0; i < Dim - 1; i++) {
			if(strides_[i + 1] == shape_[i] * strides_[i]) contiguous_length_ *= shape_[i + 1];
			else break;
		}
	
	}
}


template<std::size_t Dim, typename T>
std::ptrdiff_t ndarray_view<Dim, T>::fix_coordinate_(std::ptrdiff_t c, std::ptrdiff_t dimension) const {
	std::ptrdiff_t n = shape_[dimension];
	if(c < 0) c = n + c;
	if(c >= n) throw std::out_of_range("coordinate is out of range");
	return c;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::index_to_coordinates(const index_type& index) const -> coordinates_type {
	coordinates_type coord;
	index_type remainder = index;
	std::size_t factor = shape_.product();
	
	auto iteration = [&](std::ptrdiff_t i) {
		factor /= shape_[i];
		auto div = std::div(remainder, factor);
		coord[i] = div.quot;
		remainder = div.rem;
	};
	
	if(order_ == ndarray_order::row_major)
		for(std::ptrdiff_t i = 0; i < Dim; ++i) iteration(i);
	else
		for(std::ptrdiff_t i = Dim - 1; i >= 0; --i) iteration(i);
	
	return coord;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::coordinates_to_index(const coordinates_type& coord) const -> index_type {
	std::ptrdiff_t index = 0;
	std::ptrdiff_t factor = 1;
	
	auto iteration = [&](std::ptrdiff_t i) {
		index += factor * coord[i];
		factor *= shape_[i];
	};
	
	if(order_ == ndarray_order::row_major)
		for(std::ptrdiff_t i = Dim - 1; i >= 0; --i) iteration(i);
	else
		for(std::ptrdiff_t i = 0; i < Dim; ++i) iteration(i);

	return index;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::coordinates_to_pointer(const coordinates_type& coord) const -> pointer {
	pointer ptr = start_;
	for(std::ptrdiff_t i = 0; i < Dim; ++i)
		ptr = advance_raw_ptr(ptr, strides_[i] * coord[i]);
	return ptr;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::at(const coordinates_type& coord) const -> reference {
	coordinates_type real_coord;
	for(std::ptrdiff_t i = 0; i < Dim; ++i) real_coord[i] = fix_coordinate_(coord[i], i);
	return *coordinates_to_pointer(real_coord);
}


template<std::size_t Dim, typename T>
inline auto ndarray_view<Dim, T>::begin() const -> iterator {
	return iterator(*this, 0, start_);
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::end() const -> iterator {
	std::ptrdiff_t first_coord = (order_ == ndarray_order::row_major) ? 0 : (Dim - 1);
	index_type end_index = shape_.product();
	pointer end_pointer = advance_raw_ptr(start_, shape_[first_coord] * strides_[first_coord]);
	return iterator(*this, end_index, end_pointer);
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::section_(std::ptrdiff_t i, std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const -> ndarray_view {
	using std::swap;

	if(start < 0) start = shape_[i] + start;
	if(end < 0) end = shape_[i] + end;
	
	if(start < 0 || start >= shape_[i]) throw std::out_of_range("section start out of range");
	if(end < 0 || end > shape_[i]) throw std::out_of_range("section end out of range");
	if(start >= end) throw std::invalid_argument("section start must be lower than end");
	if(step == 0) throw std::invalid_argument("section step must not be zero");	
	
	pointer new_start = advance_raw_ptr(start_, strides_[i] * start);
	shape_type new_shape = shape_;
	strides_type new_strides = strides_;
	
	std::ptrdiff_t n = end - start;
	new_strides[i] = strides_[i] * step;
	
	if(step > 0) {
		new_shape[i] = 1 + ((n - 1) / step);
	} else {
		new_shape[i] = 1 + ((n - 1) / -step);
		new_start = advance_raw_ptr(new_start, -new_strides[i] * (new_shape[i] - 1));
	}

	return ndarray_view(new_start, new_shape, order_, new_strides);
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::section(const coordinates_type& start, const coordinates_type& end, const strides_type& steps) const -> ndarray_view {
	ndarray_view new_view = *this;
	for(std::ptrdiff_t i = 0; i < Dim; ++i)
		new_view = new_view.section_(i, start[i], end[i], steps[i]);
	return new_view;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const -> ndarray_view<Dim - 1, T> {
	return ndarray_view<Dim - 1, T>(
		advance_raw_ptr(start_, strides_[dimension] * fix_coordinate_(c, dimension)),
		shape_.erase(dimension),
		order_,
		strides_.erase(dimension)
	);
}


}