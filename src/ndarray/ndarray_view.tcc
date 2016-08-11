/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <type_traits>

#include <iostream>

namespace mf {

template<std::size_t Dim, typename T>
constexpr std::size_t ndarray_view<Dim, T>::dimension;


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::default_strides(const shape_type& shape, std::size_t padding) -> strides_type {
	Assert(is_multiple_of(padding, alignof(T)));
	strides_type strides;
	strides[Dim - 1] = sizeof(T) + padding;
	for(std::ptrdiff_t i = Dim - 1; i > 0; --i)
		strides[i - 1] = strides[i] * shape[i];
	return strides;
}



template<std::size_t Dim, typename T>
bool ndarray_view<Dim, T>::has_default_strides(std::ptrdiff_t minimal_dimension) const noexcept {
	if(strides_.back() < sizeof(T)) return false;
	for(std::ptrdiff_t i = Dim - 2; i >= minimal_dimension; --i) {
		std::ptrdiff_t expected_stride = shape_[i + 1] * strides_[i + 1];
		if(strides_[i] != expected_stride) return false;
	}
	return true;
}


template<std::size_t Dim, typename T>
bool ndarray_view<Dim, T>::has_default_strides_without_padding(std::ptrdiff_t minimal_dimension) const noexcept {
	if(has_default_strides(minimal_dimension)) return (default_strides_padding(minimal_dimension) == 0);
	else return false;
}




template<std::size_t Dim, typename T>
std::size_t ndarray_view<Dim, T>::default_strides_padding(std::ptrdiff_t minimal_dimension) const {
	Assert(has_default_strides(minimal_dimension));
	return (strides_.back() - sizeof(T));
}




template<std::size_t Dim, typename T>
ndarray_view<Dim, T>::ndarray_view(pointer start, const shape_type& shape) :
	ndarray_view(start, shape, default_strides(shape)) { }



template<std::size_t Dim, typename T>
ndarray_view<Dim, T>::ndarray_view(pointer start, const shape_type& shape, const strides_type& strides) :
	start_(start), shape_(shape), strides_(strides)
{
	std::ptrdiff_t i;
	contiguous_length_ = shape_.back();	
	for(i = Dim - 1; i > 0; i--) {
		if(strides_[i - 1] == shape_[i] * strides_[i]) contiguous_length_ *= shape_[i - 1];
		else break;
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
auto ndarray_view<Dim, T>::index_to_coordinates(index_type index) const -> coordinates_type {	
	if(index == 0) return coordinates_type(0);
	coordinates_type coord;
	index_type remainder = index;
	std::size_t factor = shape_.tail().product();
	for(std::ptrdiff_t i = 0; i < Dim - 1; ++i) {
		auto div = std::div(remainder, factor);
		coord[i] = div.quot;
		remainder = div.rem;
		factor /= shape_[i + 1];
	}
	coord.back() = remainder;
	return coord;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::coordinates_to_index(const coordinates_type& coord) const -> index_type {
	std::ptrdiff_t index = coord.back();
	std::ptrdiff_t factor = shape_[Dim - 1];
	for(std::ptrdiff_t i = Dim - 2; i >= 0; --i) {
		index += factor * coord[i];
		factor *= shape_[i];
	}
	return index;
}


template<std::size_t Dim, typename T>
void ndarray_view<Dim, T>::reset(const ndarray_view& other) noexcept {
	start_ = other.start_;
	shape_ = other.shape_;
	strides_ = other.strides_;
	contiguous_length_ = other.contiguous_length_;
}


template<std::size_t Dim, typename T> template<typename T2>
void ndarray_view<Dim, T>::assign(const ndarray_view<Dim, T2>& other) const {
	// converting assignment
	Assert_crit(shape() == other.shape(), "ndarray_view must have same shape for assignment");
	if(shape().product() == 0) return;
	std::copy(other.begin(), other.end(), begin());
}


template<std::size_t Dim, typename T>
void ndarray_view<Dim, T>::assign(const ndarray_view<Dim, const T>& other) const {
	// assignment without conversion
	
	Assert_crit(shape() == other.shape(), "ndarray_view must have same shape for assignment");
	if(shape().product() == 0) return;
	
	if(std::is_pod<T>::value && strides() == other.strides() && has_default_strides()) {
		// optimize when possible
		const ndarray_format& array_format = format(*this);
		Assert_crit(array_format == format(other));
		ndarray_frame_copy(static_cast<void*>(start()), static_cast<const void*>(start()), array_format);
	} else {
		std::copy(other.begin(), other.end(), begin());
	}
}

// TODO comparison optimization

template<std::size_t Dim, typename T> template<typename T2>
bool ndarray_view<Dim, T>::compare(const ndarray_view<Dim, T2>& other) const {
	if(shape() != other.shape()) return false;
	else return std::equal(other.begin(), other.end(), begin());
}


template<std::size_t Dim, typename T>
bool ndarray_view<Dim, T>::compare(const ndarray_view<Dim, const T>& other) const {
	if(shape() != other.shape()) return false;
	else if(same(*this, other)) return true;
	else return std::equal(other.begin(), other.end(), begin());
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
	index_type end_index = shape().product();
	coordinates_type end_coord = index_to_coordinates(end_index);
	return iterator(*this, end_index, coordinates_to_pointer(end_coord));
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

	return ndarray_view(new_start, new_shape, new_strides);
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::section(const coordinates_type& start, const coordinates_type& end, const strides_type& steps) const -> ndarray_view {
	ndarray_view new_view = *this;
	for(std::ptrdiff_t i = 0; i < Dim; ++i) {
		auto sec = new_view.section_(i, start[i], end[i], steps[i]);
		new_view.reset(sec);
	}
	return new_view;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const -> ndarray_view<Dim - 1, T> {
	return ndarray_view<Dim - 1, T>(
		advance_raw_ptr(start_, strides_[dimension] * fix_coordinate_(c, dimension)),
		shape_.erase(dimension),
		strides_.erase(dimension)
	);
}


template<std::size_t Dim, typename T>
ndarray_view<1 + Dim, T> ndarray_view<Dim, T>::add_front_axis() const {
	auto new_shape = ndcoord_cat(1, shape());
	auto new_strides = ndcoord_cat(0, strides());
	return ndarray_view<1 + Dim, T>(start(), new_shape, new_strides);
}

template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::swapaxis(std::size_t axis1, std::size_t axis2) const -> ndarray_view {
	if(axis1 >= Dim || axis2 >= Dim) throw std::invalid_argument("axis index out of range");
	auto new_strides = strides_;
	std::swap(new_strides[axis1], new_strides[axis2]);
	auto new_shape = shape_;
	std::swap(new_shape[axis1], new_shape[axis2]);
	return ndarray_view(start_, new_shape, new_strides);
}


template<std::size_t Dim, typename T, std::size_t New_dim>
ndarray_view<New_dim, T> reshape(const ndarray_view<Dim, T>& vw, const ndsize<New_dim>& new_shape) {
	if(vw.strides() != vw.default_strides(vw.shape())) throw std::logic_error("can reshape only with default strides");
	if(new_shape.product() != vw.shape().product()) throw std::invalid_argument("new shape must have same product");
	return ndarray_view<New_dim, T>(vw.start(), new_shape);
}


template<std::size_t Dim, typename T>
ndarray_view<1, T> flatten(const ndarray_view<Dim, T>& vw) {
	auto new_shape = make_ndsize(vw.shape().product());
	return reshape(vw, new_shape);
}

}
