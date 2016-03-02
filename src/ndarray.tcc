#include <algorithm>

namespace mf {

template<std::size_t Dim, typename T, typename Allocator>
auto ndarray<Dim, T, Allocator>::strides_with_padding_(const shape_type& shape, const padding_type& padding) -> strides_type {
	strides_type new_strides = base::default_strides(shape);
	new_strides += strides_type(padding * padding_type(sizeof(T)));
	return new_strides;
}


template<std::size_t Dim, typename T, typename Allocator>
void ndarray<Dim, T, Allocator>::allocate_() {
	std::size_t n = 1;
	for(std::ptrdiff_t i = Dim - 1; i >= 0; --i) {
		n += padding_[i];
		n *= base::shape_[i];
	}
	
	T* ptr = std::allocator_traits<Allocator>::allocate(allocator_, n);
	this->start_ = ptr;
	allocated_size_ = n;
}


template<std::size_t Dim, typename T, typename Allocator>
void ndarray<Dim, T, Allocator>::deallocate_() {
	std::allocator_traits<Allocator>::deallocate(allocator_, base::start_, allocated_size_);
	this->start_ = nullptr;
	allocated_size_ = 0;
}


template<std::size_t Dim, typename T, typename Allocator>
ndarray<Dim, T, Allocator>::ndarray(const shape_type& shape, const padding_type& padding, const Allocator& alloc) :
	base(nullptr, shape, strides_with_padding_(shape, padding)),
	allocator_(alloc),
	padding_(padding)
{
	allocate_();
}


template<std::size_t Dim, typename T, typename Allocator>	
ndarray<Dim, T, Allocator>::ndarray(const const_view_type& in) :
	ndarray(in.shape())
{
	std::copy(in.begin(), in.end(), begin());
}


template<std::size_t Dim, typename T, typename Allocator>	
ndarray<Dim, T, Allocator>::~ndarray() {
	deallocate_();
}


template<std::size_t Dim, typename T, typename Allocator>	
auto ndarray<Dim, T, Allocator>::operator=(const const_view_type& in) -> ndarray& {
	if(in.shape().product() != base::shape().product()) {
		deallocate_();
		base::shape_ = in.shape();
		base::strides_ = strides_with_padding_(base::shape_, padding_);
		allocate_();
	}
	std::copy(in.begin(), in.end(), begin());
	return *this;
}


}
