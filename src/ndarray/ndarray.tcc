namespace mf {

template<std::size_t Dim, typename T, typename Allocator>
auto ndarray<Dim, T, Allocator>::strides_with_padding_(const shape_type& shp, const padding_type& padding) -> strides_type {
	return view_type::default_strides(shp) + strides_type(padding);
}


template<std::size_t Dim, typename T, typename Allocator>
void ndarray<Dim, T, Allocator>::allocate_() {
	std::size_t n = 1;
	for(std::ptrdiff_t i = Dim - 1; i >= 0; --i) {
		n += padding_[i];
		n *= shape()[i];
	}
	
	T* ptr = std::allocator_traits<Allocator>::allocate(allocator_, n);

	view_.reset(
		ptr,
		view_.shape(),
		strides_with_padding_(shape(), padding_)
	);
	allocated_size_ = n;
}


template<std::size_t Dim, typename T, typename Allocator>
void ndarray<Dim, T, Allocator>::deallocate_() {
	std::allocator_traits<Allocator>::deallocate(allocator_, start(), allocated_size_);
	allocated_size_ = 0;
}


template<std::size_t Dim, typename T, typename Allocator>
ndarray<Dim, T, Allocator>::ndarray(const shape_type& shp, const padding_type& padding, const Allocator& alloc) :
	allocator_(alloc),
	padding_(padding),
	view_(nullptr, shp, strides_with_padding_(shp, padding))
{
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
