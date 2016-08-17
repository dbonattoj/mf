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

#ifndef MF_NDARRAY_ITERATOR_H_
#define MF_NDARRAY_ITERATOR_H_

#include <iterator>

namespace mf {

template<std::size_t Dim, typename Elem>
class ndarray_view;

/// Random access iterator which traverses an \ref ndarray_view.
/** Always traverses the elements in order of increasing index, regardless of strides. Random-access iterator operations
 ** (addition, comparation, etc.) act on index values. Index and coordinates of current item can be accessed using
 ** index() and coordinates().
 ** Incrementation and decrementation are optimized when the strides of the `ndarray_view` are (partially) default, i.e.
 ** items with sequential indices have sequential memory addresses, possibly with padding inbetween. This can be for the
 ** entire `ndarray_view`, or just for smaller segments at a time.
 ** If `coordinates()` is called at each iteration, using `ndspan_iterator` may be more efficient because it does not
 ** recompute the coordinates from an index each time. */
template<std::size_t Dim, typename Elem>
class ndarray_iterator :
public std::iterator<std::random_access_iterator_tag, Elem> {
	using base = std::iterator<std::random_access_iterator_tag, Elem>;

public:
	using view_type = ndarray_view<Dim, Elem>;

	using typename base::value_type;
	using typename base::reference;
	using typename base::pointer;
	
	using index_type = typename view_type::index_type;
	using coordinates_type = typename view_type::coordinates_type;

	constexpr static std::size_t dimension = view_type::dimension;

private:
	const view_type view_;
	pointer pointer_ = nullptr;
	index_type index_ = 0;
	std::ptrdiff_t pointer_step_;
	std::ptrdiff_t contiguous_length_;
	
	void forward_(std::ptrdiff_t);
	void backward_(std::ptrdiff_t);

public:
	ndarray_iterator() = default;
	ndarray_iterator(const view_type& vw, index_type index, pointer ptr);
	ndarray_iterator(const ndarray_iterator&) = default;
	
	ndarray_iterator& operator=(const ndarray_iterator&);
	
	ndarray_iterator& operator++();
	ndarray_iterator operator++(int);
	ndarray_iterator& operator--();
	ndarray_iterator operator--(int);
	
	ndarray_iterator& operator+=(std::ptrdiff_t);
	ndarray_iterator& operator-=(std::ptrdiff_t);
	
	reference operator*() const noexcept { return *pointer_; }
	pointer operator->() const noexcept { return pointer_; }
	pointer ptr() const noexcept { return pointer_; }
	
	reference operator[](std::ptrdiff_t n) const { return *(*this + n); }
	
	friend bool operator==(const ndarray_iterator& a, const ndarray_iterator& b) noexcept
		{ return a.index() == b.index(); }
	friend bool operator!=(const ndarray_iterator& a, const ndarray_iterator& b) noexcept
		{ return a.index() != b.index(); }
	friend bool operator<(const ndarray_iterator& a, const ndarray_iterator& b) noexcept
		{ return a.index() < b.index(); }
	friend bool operator<=(const ndarray_iterator& a, const ndarray_iterator& b) noexcept
		{ return a.index() <= b.index(); }
	friend bool operator>(const ndarray_iterator& a, const ndarray_iterator& b) noexcept
		{ return a.index() > b.index(); }
	friend bool operator>=(const ndarray_iterator& a, const ndarray_iterator& b) noexcept
		{ return a.index() >= b.index(); }
	
	friend ndarray_iterator operator+(const ndarray_iterator& it, std::ptrdiff_t n)
		{ auto copy = it; copy += n; return copy; }
	friend ndarray_iterator operator+(std::ptrdiff_t n, const ndarray_iterator& it)
		{ auto copy = it; copy += n; return copy; }
	friend ndarray_iterator operator-(const ndarray_iterator& it, std::ptrdiff_t n)
		{ auto copy = it; copy -= n; return copy; }
	friend std::ptrdiff_t operator-(const ndarray_iterator& a, const ndarray_iterator& b)
		{ return a.index() - b.index(); }
	
	index_type index() const noexcept { return index_; }
	coordinates_type coordinates() const noexcept { return view_.index_to_coordinates(index_); }
};


template<std::size_t Dim, typename Elem>
constexpr std::size_t ndarray_iterator<Dim, Elem>::dimension;


}

#include "ndarray_iterator.tcc"

#endif
