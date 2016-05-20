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

#ifndef MF_NDARRAY_H_
#define MF_NDARRAY_H_

#include "ndarray_view.h"
#include "ndcoord.h"
#include "../os/memory.h"
#include <memory>
#include <utility>

#define MF_NDARRAY_VIEW_FUNC_(func) \
	template<typename... Args> decltype(auto) func(Args&&... args) { \
		return view().func(std::forward<Args>(args)...); \
	} \
	template<typename... Args> decltype(auto) func(Args&&... args) const { \
		return cview().func(std::forward<Args>(args)...); \
	}


namespace mf {

/// Multidimensional array container.
/** Based on `ndarray_view`, but also owns items. Allocates and deallocates memory. Const-correct interface.
 ** Uses default strides when created from public constructor. Derived classes (i.e. \ref ring) can create \ref ndarray
 ** with padding between elements (last dimension), and padding between frames (first dimension). */
template<std::size_t Dim, typename T, typename Allocator = raw_allocator>
class ndarray {
public:
	using view_type = ndarray_view<Dim, T>;
	using const_view_type = ndarray_view<Dim, const T>;

	using index_type = typename view_type::index_type;
	using coordinates_type = typename view_type::coordinates_type;
	using shape_type = typename view_type::shape_type;
	using strides_type = typename view_type::strides_type;
	using span_type = typename view_type::span_type;

	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
		
	using iterator = typename view_type::iterator;
	using const_iterator = typename const_view_type::iterator;

	constexpr static std::size_t dimension = Dim;

protected:
	Allocator allocator_; ///< Raw allocator used to allocate memory, in bytes.
	std::size_t stride_; ///< Stride of elements, in bytes.
	std::size_t padding_; ///< Padding between frames, in bytes.
	std::size_t allocated_size_ = 0; ///< Allocated memory size, in bytes.
	view_type view_; ///< View used to access items.

	void allocate_();
	void deallocate_();
	
	/// Constructor for use by derived classes.
	/** Size of allocated memory segment is `shape.front() * (padding + shape.tail().product()*stride)`.
	 ** Caller must ensure this satisfies requirements of allocator.
	 ** \param shape Shape of entire array. First component is length of array, remaining components is frame shape.
	 ** \param padding Padding between frames, in bytes.
	 ** \param stride Stride of elements, in bytes. Must be multiple of alignof(T) and leq to sizeof(T).
	 ** \param allocator Allocator instance. */
	ndarray(const shape_type& shape, std::size_t padding, std::size_t stride, const Allocator& allocator);

	strides_type strides_with_padding_(const shape_type& shape, std::size_t padding);

public:
	explicit ndarray(const shape_type& shp, const Allocator& allocator = Allocator()) :
		ndarray(shp, 0, sizeof(T), allocator) { }

	explicit ndarray(const const_view_type&);
	ndarray(const ndarray& arr) : ndarray(arr.cview()) { }

	~ndarray();
	
	ndarray& operator=(const const_view_type& arr);
	ndarray& operator=(const ndarray& arr) { return operator=(arr.cview()); }
	
	view_type view() { return view_; }
	const_view_type view() const { return cview(); }
	const_view_type cview() const { return const_view_type(view_);  }
	
	operator view_type () noexcept { return view(); }
	operator const_view_type () const noexcept { return cview(); }
	
	coordinates_type index_to_coordinates(const index_type& index) const
		{ return view_.index_to_coordinates(index); }
	index_type coordinates_to_index(const coordinates_type& coord) const
		{ return view_.coordinates_to_index(coord); }
	pointer coordinates_to_pointer(const coordinates_type& coord)
		{ return view_.coordinates_to_pointer(coord); }
	const_pointer coordinates_to_pointer(const coordinates_type& coord) const
		{ return view_.coordinates_to_pointer(coord); }
	
	std::size_t size() const noexcept { return view_.size(); }
	
	pointer start() noexcept { return view_.start(); }
	const_pointer start() const noexcept { return view_.start(); }
	const shape_type& shape() const noexcept { return view_.shape(); }
	const strides_type& strides() const noexcept { return view_.strides(); }
	std::size_t contiguous_length() const noexcept { return view_.contiguous_length(); }
	std::size_t padding() const noexcept { return padding_; }

	MF_NDARRAY_VIEW_FUNC_(section);
	MF_NDARRAY_VIEW_FUNC_(slice)
	MF_NDARRAY_VIEW_FUNC_(operator());
	MF_NDARRAY_VIEW_FUNC_(operator[]);
	MF_NDARRAY_VIEW_FUNC_(at);
		
	iterator begin() { return view_.begin(); }
	const_iterator begin() const { return cview().begin(); }
	const_iterator cbegin() const { return cview().begin(); }
	iterator end() { return view_.end(); }
	const_iterator end() const { return cview().end(); }
	const_iterator cend() const { return cview().end(); }
};



template<std::size_t Dim, typename T>
auto make_ndarray(const ndarray_view<Dim, T>& vw) {
	return ndarray<Dim, std::remove_const_t<T>>(vw);
}


}

#include "ndarray.tcc"

#endif
