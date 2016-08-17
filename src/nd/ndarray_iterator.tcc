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

#include <cassert>
#include "../utility/misc.h"

namespace mf {


template<std::size_t Dim, typename Elem>
inline void ndarray_iterator<Dim, Elem>::forward_(std::ptrdiff_t d) {
	Expects_crit(d >= 0);
	std::ptrdiff_t contiguous_limit = contiguous_length_ - (index_ % contiguous_length_);
	index_ += d;
	if(d < contiguous_limit) {
		pointer_ = advance_raw_ptr(pointer_, d * pointer_step_);
	} else {
		auto new_coord = view_.index_to_coordinates(index_);
		pointer_ = view_.coordinates_to_pointer(new_coord);
	}
}


template<std::size_t Dim, typename Elem>
inline void ndarray_iterator<Dim, Elem>::backward_(std::ptrdiff_t d) {
	Expects_crit(d >= 0);
	std::ptrdiff_t contiguous_limit = index_ % contiguous_length_;
	index_ -= d;
	if(d <= contiguous_limit) {
		pointer_ = advance_raw_ptr(pointer_, -d * pointer_step_);
	} else {
		auto new_coord = view_.index_to_coordinates(index_);
		pointer_ = view_.coordinates_to_pointer(new_coord);
	}
}


template<std::size_t Dim, typename Elem>
ndarray_iterator<Dim, Elem>::ndarray_iterator(const view_type& vw, index_type index, pointer ptr) :
	view_(vw),
	pointer_(ptr),
	index_(index),
	pointer_step_(vw.strides().back()),
	contiguous_length_(vw.contiguous_length()) { }


template<std::size_t Dim, typename Elem>
auto ndarray_iterator<Dim, Elem>::operator=(const ndarray_iterator& it) -> ndarray_iterator& {
	pointer_ = it.pointer_;
	index_ = it.index_;
	pointer_step_ = it.pointer_step_;
	contiguous_length_ = it.contiguous_length_;
	return *this;
}


template<std::size_t Dim, typename Elem>
auto ndarray_iterator<Dim, Elem>::operator++() -> ndarray_iterator& {
	forward_(1);
	return *this;
}


template<std::size_t Dim, typename Elem>
auto ndarray_iterator<Dim, Elem>::operator++(int) -> ndarray_iterator {
	auto copy = *this;
	forward_(1);
	return copy;
}


template<std::size_t Dim, typename Elem>
auto ndarray_iterator<Dim, Elem>::operator--() -> ndarray_iterator& {
	backward_(1);
	return *this;
}


template<std::size_t Dim, typename Elem>
auto ndarray_iterator<Dim, Elem>::operator--(int) -> ndarray_iterator {
	auto copy = *this;
	backward_(1);
	return copy;
}


template<std::size_t Dim, typename Elem>
auto ndarray_iterator<Dim, Elem>::operator+=(std::ptrdiff_t n) -> ndarray_iterator& {
	if(n > 0) forward_(n);
	else backward_(-n);
	return *this;
}


template<std::size_t Dim, typename Elem>
auto ndarray_iterator<Dim, Elem>::operator-=(std::ptrdiff_t n) -> ndarray_iterator& {
	if(n > 0) backward_(n);
	else forward_(-n);
	return *this;
}


}
