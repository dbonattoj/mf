#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace mf {
 
template<std::size_t Dim, typename T, typename Allocator>
auto ndarray<Dim, T, Allocator>::strides_with_padding_(const shape_type& shp, std::size_t padding) -> strides_type {
	std::size_t element_padding = stride_ - sizeof(T);
	strides_type str = view_type::default_strides(shp, element_padding);
	str.front() += padding;
	return str;
}


template<std::size_t Dim, typename T, typename Allocator>
void ndarray<Dim, T, Allocator>::allocate_() {
	MF_ASSERT(is_nonzero_multiple_of(stride_, alignof(T)));
	
	std::size_t frame_bytes = shape().tail().product() * stride_;
	std::size_t bytes = (frame_bytes + padding_) * shape().front();
	
	if(! is_nonzero_multiple_of(frame_bytes + padding_, alignof(T)))
		throw std::runtime_error("padding does not satisfy alignment constraint of element type");
		
	void* raw_ptr = allocator_.raw_allocate(bytes, stride_);
	if(! is_nonzero_multiple_of(reinterpret_cast<std::uintptr_t>(raw_ptr), stride_))
		throw std::runtime_error("allocated memory not properly aligned");

	T* ptr = reinterpret_cast<T*>(raw_ptr);
	
	view_.reset(
		ptr,
		view_.shape(),
		strides_with_padding_(shape(), padding_)
	);
	allocated_size_ = bytes;
}


template<std::size_t Dim, typename T, typename Allocator>
void ndarray<Dim, T, Allocator>::deallocate_() {
	void* raw_ptr = reinterpret_cast<void*>(view_.start());
	allocator_.raw_deallocate(raw_ptr, allocated_size_);
	allocated_size_ = 0;
}


template<std::size_t Dim, typename T, typename Allocator>
ndarray<Dim, T, Allocator>::ndarray(const shape_type& shp, std::size_t padding, std::size_t stride, const Allocator& alloc) :
	allocator_(alloc),
	stride_(stride),
	padding_(padding),
	view_(nullptr, shp, strides_with_padding_(shp, padding)) // TODO stub ndarray_view instead
{		
	if(! is_nonzero_multiple_of(stride_, alignof(T))) throw std::invalid_argument("specified alignment must be multiple of alignof(T)");
	allocate_();
}


template<std::size_t Dim, typename T, typename Allocator>	
ndarray<Dim, T, Allocator>::ndarray(const const_view_type& in) :
	ndarray(in.shape())
{
	view_.assign(in);
}


template<std::size_t Dim, typename T, typename Allocator>	
ndarray<Dim, T, Allocator>::~ndarray() {
	deallocate_();
}


template<std::size_t Dim, typename T, typename Allocator>	
auto ndarray<Dim, T, Allocator>::operator=(const const_view_type& in) -> ndarray& {
	if(in.shape().product() != shape().product()) {
		deallocate_();
		view_.reset(view().start(), in.shape(), strides_with_padding_(in.shape(), padding_));
		allocate_();
	}
	view_.assign(in);
	return *this;
}


}
