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

#ifndef MF_NDARRAY_OPAQUE_ITERATOR_H_
#define MF_NDARRAY_OPAQUE_ITERATOR_H_

#include "../ndarray_iterator.h"
#include <type_traits>

namespace mf {

template<std::size_t Dim, bool Mutable>
class ndarray_view_opaque;

template<std::size_t Dim, bool Mutable>
class ndarray_opaque_iterator : public ndarray_iterator<Dim + 1, std::conditional_t<Mutable, byte, const byte>> {
	using base = ndarray_iterator<Dim + 1, std::conditional_t<Mutable, byte, const byte>>;

public:
	using view_type = ndarray_view_opaque<Dim, Mutable>;
	using coordinates_type = typename view_type::coordinates_type;

	constexpr static std::size_t dimension = view_type::dimension;

	using frame_ptr = std::conditional_t<Mutable, void*, const void*>;
	using pointer = frame_ptr;

	ndarray_opaque_iterator() : base() { }
	explicit ndarray_opaque_iterator(const base& it) : base(it) { }
	ndarray_opaque_iterator(const ndarray_opaque_iterator&) = default;
	ndarray_opaque_iterator& operator=(const ndarray_opaque_iterator&) = default;
	
	pointer ptr() const noexcept { return base::ptr(); }
	coordinates_type coordinates() const noexcept { return head(base::coordinates()); }

	typename base::reference operator*() const noexcept = delete;
	pointer operator->() const noexcept = delete;
	typename base::reference operator[](std::ptrdiff_t n) const = delete;
};


template<std::size_t Dim, bool Mutable>
constexpr std::size_t ndarray_opaque_iterator<Dim, Mutable>::dimension;


}

#endif
