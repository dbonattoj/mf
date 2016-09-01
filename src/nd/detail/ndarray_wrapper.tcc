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

#include <iostream>

namespace mf { namespace detail {

template<typename View, typename Const_view, typename Allocator>
void ndarray_wrapper<View, Const_view, Allocator>::allocate_(std::size_t size, std::size_t alignment) {
	if(size > 0) {
		void* buf = allocator_.raw_allocate(size, alignment);
		allocated_size_ = size;
		allocated_buffer_ = buf;
	}
}


template<typename View, typename Const_view, typename Allocator>
void ndarray_wrapper<View, Const_view, Allocator>::deallocate_() {
	if(allocated_size_ != 0) {
		allocator_.raw_deallocate(allocated_buffer_, allocated_size_);
		allocated_size_ = 0;
		allocated_buffer_ = nullptr;
	}
}


template<typename View, typename Const_view, typename Allocator> template<typename... Arg>
ndarray_wrapper<View, Const_view, Allocator>::ndarray_wrapper(
	const shape_type& shape,
	const strides_type& strides,
	std::size_t allocate_size,
	std::size_t allocate_alignment,
	const Allocator& allocator,
	const Arg&... view_arguments
) :
	allocator_(allocator)
{
	allocate_(allocate_size, allocate_alignment);
	view_.reset(view_type(
		static_cast<typename view_type::pointer>(allocated_buffer_),
		shape,
		strides,
		view_arguments...
	));
	Assert(static_cast<void*>(view_.start()) == allocated_buffer_, "first element in ndarray must be at buffer start");
}
	

template<typename View, typename Const_view, typename Allocator>
ndarray_wrapper<View, Const_view, Allocator>::ndarray_wrapper(ndarray_wrapper&& arr) :
	allocator_(),
	allocated_size_(arr.allocated_size_),
	allocated_buffer_(arr.allocated_buffer_),
	view_(arr.view_)
{
	arr.view_.reset();
	arr.allocated_size_ = 0;
	arr.allocated_buffer_ = nullptr;
}
		

template<typename View, typename Const_view, typename Allocator>
ndarray_wrapper<View, Const_view, Allocator>::~ndarray_wrapper() {
	deallocate_();
}


template<typename View, typename Const_view, typename Allocator>
auto ndarray_wrapper<View, Const_view, Allocator>::operator=(ndarray_wrapper&& arr) -> ndarray_wrapper& {
	if(&arr == this) return *this;
	
	deallocate_();
	
	allocated_size_ = arr.allocated_size_;
	allocated_buffer_ = arr.allocated_buffer_;
	view_.reset(arr.view_);
	
	arr.allocated_size_ = 0;
	arr.allocated_buffer_ = nullptr;
	arr.view_.reset();
	
	return *this;
}


template<typename View, typename Const_view, typename Allocator> template<typename... Arg>
void ndarray_wrapper<View, Const_view, Allocator>::reset_(
	const shape_type& shape,
	const strides_type& strides,
	std::size_t allocate_size,
	std::size_t allocate_alignment,
	const Arg&... view_arguments
) {	
	// reallocate memory only if necessary
	if(allocate_size != allocated_size() || ! is_aligned(allocated_buffer_, allocate_alignment)) {
		deallocate_();
		allocate_(allocate_size, allocate_alignment);
	}
	
	view_.reset(view_type(
		static_cast<typename view_type::pointer>(allocated_buffer_),
		shape,
		strides,
		view_arguments...
	));
}


}}
