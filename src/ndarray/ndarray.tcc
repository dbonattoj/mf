#include <utility>

namespace mf {
	
template<std::size_t Dim, typename Elem, typename Allocator>
ndarray<Dim, Elem, Allocator>::ndarray(const Allocator& allocator) :
	base(allocator) { }


template<std::size_t Dim, typename Elem, typename Allocator>
ndarray<Dim, Elem, Allocator>::ndarray(const shape_type& shape, std::size_t elem_padding, const Allocator& allocator) :
base(
	shape,
	view_type::default_strides(shape, elem_padding),
	(sizeof(Elem) + elem_padding) * shape.product(),
	alignof(Elem),
	allocator
) { }
	

template<std::size_t Dim, typename Elem, typename Allocator> template<typename Other_elem>
ndarray<Dim, Elem, Allocator>::ndarray
(const ndarray_view<Dim, Other_elem>& vw, std::size_t elem_padding, const Allocator& allocator) :
base(
	vw.shape(),
	view_type::default_strides(vw.shape, elem_padding),
	(sizeof(Elem) + elem_padding) * vw.shape().product(),
	alignof(Elem),
	allocator
) {
	base::view().assign(vw);
}


template<std::size_t Dim, typename Elem, typename Allocator>
ndarray<Dim, Elem, Allocator>::ndarray(const ndarray& arr) :
base(
	arr.shape(),
	arr.strides(),
	arr.allocated_byte_size(),
	alignof(Elem),
	arr.get_allocator()
) {
	base::view().assign(arr.cview());
}


template<std::size_t Dim, typename Elem, typename Allocator>
ndarray<Dim, Elem, Allocator>::ndarray(ndarray&& arr) :
base(std::move(arr)) { }


template<std::size_t Dim, typename Elem, typename Allocator> template<typename Other_elem>
void ndarray<Dim, Elem, Allocator>::assign(const ndarray_view<Dim, Other_elem>& vw, std::size_t elem_padding) {
	base::reset_(
		vw.shape(),
		view_type::default_strides(vw.shape(), elem_padding),
		(sizeof(Elem) + elem_padding) * vw.shape().product(),
		alignof(Elem)
	);
	base::view().assign(vw);
}


template<std::size_t Dim, typename Elem, typename Allocator>
auto ndarray<Dim, Elem, Allocator>::operator=(const ndarray& arr) -> ndarray& {
	if(&arr == this) return *this;
	base::reset_(
		arr.shape(),
		arr.strides(),
		arr.allocated_size(),
		alignof(Elem)
	);
	base::view().assign(vw);
	return *this;
}

	
template<std::size_t Dim, typename Elem, typename Allocator>
auto ndarray<Dim, Elem, Allocator>::operator=(ndarray&& arr) -> ndarray& {
	base::operator=(std::move(arr));
	return *this;
}




}

