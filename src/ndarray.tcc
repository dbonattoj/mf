#include <algorithm>

namespace mf {

template<std::size_t Dim, typename T, typename Allocator>
ndarray<Dim, T, Allocator>::ndarray(const shape_type& shape, const Allocator& alloc) :
	base(nullptr, shape),
	allocator_(alloc),
	length_(shape.product() * sizeof(T))
{
	base::start_ = allocator_.allocate(base::shape_.product());
}


template<std::size_t Dim, typename T, typename Allocator>	
ndarray<Dim, T, Allocator>::ndarray(const const_view_type& in) :
	ndarray(in.shape())
{
	std::copy(in.begin(), in.end(), raw_begin());
}


template<std::size_t Dim, typename T, typename Allocator>	
ndarray<Dim, T, Allocator>::~ndarray() {
	allocator_.deallocate(base::start_, base::shape_.product());
}


template<std::size_t Dim, typename T, typename Allocator>	
auto ndarray<Dim, T, Allocator>::operator=(const const_view_type& in) -> ndarray& {
	if(in.shape().product() != base::shape().product()) {
		allocator_.deallocate(base::start_, base::shape_.product());
		length_ = base::shape_.product() * sizeof(T);
		base::shape_ = in.shape();
		base::strides_ = base::default_strides(base::shape_);
		base::start_ = allocator_.allocate(base::shape_.product());
	}
	std::copy(in.begin(), in.end(), raw_begin());
	return *this;
}


}
