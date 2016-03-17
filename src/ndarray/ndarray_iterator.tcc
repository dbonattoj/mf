#include <cassert>
#include "../util.h"

namespace mf {


template<typename Array>
inline void ndarray_iterator<Array>::forward_(std::ptrdiff_t d) {
	assert(d >= 0);

	std::ptrdiff_t contiguous_limit = contiguous_length_ - (index_ % contiguous_length_);
	index_ += d;
	if(d < contiguous_limit) {
		pointer_ = advance_raw_ptr(pointer_, d * pointer_step_);
	} else {
		auto new_coord = array_->index_to_coordinates(index_);
		pointer_ = array_->coordinates_to_pointer(new_coord);
	}
}


template<typename Array>
inline void ndarray_iterator<Array>::backward_(std::ptrdiff_t d) {
	assert(d >= 0);

	std::ptrdiff_t contiguous_limit = index_ % contiguous_length_;
	index_ -= d;
	if(d <= contiguous_limit) {
		pointer_ = advance_raw_ptr(pointer_, -d * pointer_step_);
	} else {
		auto new_coord = array_->index_to_coordinates(index_);
		pointer_ = array_->coordinates_to_pointer(new_coord);
	}
}


template<typename Array>
ndarray_iterator<Array>::ndarray_iterator(const Array& array, index_type index, pointer ptr) :
	array_(&array),
	pointer_(ptr),
	index_(index),
	pointer_step_(array.strides().back()),
	contiguous_length_(array.contiguous_length()) { }


template<typename Array>
auto ndarray_iterator<Array>::operator=(const ndarray_iterator& it) -> ndarray_iterator& {
	pointer_ = it.pointer_;
	index_ = it.index_;
	pointer_step_ = it.pointer_step_;
	contiguous_length_ = it.contiguous_length_;
	return *this;
}


template<typename Array>
auto ndarray_iterator<Array>::operator++() -> ndarray_iterator& {
	forward_(1);
	return *this;
}


template<typename Array>
auto ndarray_iterator<Array>::operator++(int) -> ndarray_iterator {
	auto copy = *this;
	forward_(1);
	return copy;
}


template<typename Array>
auto ndarray_iterator<Array>::operator--() -> ndarray_iterator& {
	backward_(1);
	return *this;
}


template<typename Array>
auto ndarray_iterator<Array>::operator--(int) -> ndarray_iterator {
	auto copy = *this;
	backward_(1);
	return copy;
}


template<typename Array>
auto ndarray_iterator<Array>::operator+=(std::ptrdiff_t n) -> ndarray_iterator& {
	if(n > 0) forward_(n);
	else backward_(-n);
	return *this;
}


template<typename Array>
auto ndarray_iterator<Array>::operator-=(std::ptrdiff_t n) -> ndarray_iterator& {
	if(n > 0) backward_(n);
	else forward_(-n);
	return *this;
}


}