namespace mf {

template<typename View, typename Const_view, typename Allocator>
void ndarray_wrapper<View, Const_view, Allocator>::allocate_(std::size_t size, std::size_t alignment) {
	void* buf = allocator_.raw_allocate(size, alignment);
	allocated_size_ = size;
	allocated_buffer_ = buf;
}


template<typename View, typename Const_view, typename Allocator>
void ndarray_wrapper<View, Const_view, Allocator>::deallocate_() {
	if(allocated_size_ != 0) {
		allocator_.deallocate(allocated_buffer_, allocated_size_);
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
) : allocator_(allocator) {
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
ndarray_wrapper<View, Const_view, Allocator>::ndarray(const Allocator& allocator) :
	allocator_(allocator) { }
	

template<typename View, typename Const_view, typename Allocator>
ndarray_wrapper<View, Const_view, Allocator>::ndarray(ndarray_wrapper&& arr) :
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
ndarray_wrapper<View, Const_view, Allocator>::~ndarray() {
	deallocate_();
}


template<typename View, typename Const_view, typename Allocator>
auto ndarray_wrapper<View, Const_view, Allocator>::operator=(ndarray&& arr) -> ndarray_wrapper& {
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
	if(allocate_size != allocated_size_() || ! is_aligned(allocated_buffer_, allocate_alignment) {
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



}
