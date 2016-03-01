namespace mf {

template<std::size_t Dim, typename T, typename Allocator>
ndarray<Dim, T, Allocator>::ndarray(const shape_type& shape, const Allocator& alloc) :
	base(nullptr, shp),
	allocator_(alloc),
	length_(shp.product() * sizeof(T))
{
	base::start_ = allocator_.allocate(base::shape_.product());
}


template<std::size_t Dim, typename T, typename Allocator>	
ndarray<Dim, T, Allocator>::ndarray(const ndarray& arr) :
	ndarray(arr.shape())
{
	//...
}


template<std::size_t Dim, typename T, typename Allocator>	
ndarray<Dim, T, Allocator>::~ndarray() {
	allocator_.deallocate(base::start_, base::shape_.product());
}


}
