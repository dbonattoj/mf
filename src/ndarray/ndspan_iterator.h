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

#ifndef MF_NDSPAN_ITERATOR_H_
#define MF_NDSPAN_ITERATOR_H_

#include <iterator>
#include "ndspan.h"
#include "ndcoord.h"

namespace mf {

/// Iterator over all coordinates in an `ndspan`.
/** Value type of iterator is `ndcoord<Dim, T>`. Forward iterator only. Iterates over coordinates in same order as
 ** the indices in `ndarray_view`. */
template<std::size_t Dim, typename T>
class ndspan_iterator :
public std::iterator<std::forward_iterator_tag, ndcoord<Dim, T>> {
	using base = std::iterator<std::forward_iterator_tag, ndcoord<Dim, T>>;

public:
	using span_type = ndspan<Dim, T>;
	using coordinates_type = typename base::value_type;
	constexpr static std::size_t dimension = Dim;

private:
	span_type span_;
	coordinates_type coordinates_;

public:
	ndspan_iterator() = default;
	ndspan_iterator(const span_type& span, const coordinates_type& coord) : span_(span), coordinates_(coord) { }
	ndspan_iterator(const ndspan_iterator&) = default;
	ndspan_iterator& operator=(const ndspan_iterator&) = default;

	ndspan_iterator& operator++();
	
	ndspan_iterator operator++(int) {
		ndspan_iterator copy = *this;
		operator++();
		return copy;
	}
	
	coordinates_type operator*() const noexcept { return coordinates_; }
	const coordinates_type* operator->() const noexcept { return &coordinates_; }
	
	friend bool operator==(const ndspan_iterator& a, const ndspan_iterator& b) noexcept
		{ return *a == *b; }
	friend bool operator!=(const ndspan_iterator& a, const ndspan_iterator& b) noexcept
		{ return *a != *b; }
};


template<std::size_t Dim, typename T>
auto ndspan_iterator<Dim, T>::operator++() -> ndspan_iterator& {	
	std::ptrdiff_t i = Dim - 1;
	T c = ++coordinates_[i];
	bool carry = (c == span_.end_pos()[i]);
	
	while(carry && i > 0) {
		coordinates_[i] = span_.start_pos()[i];
		--i;
		c = ++coordinates_[i];
		carry = (c == span_.end_pos()[i]);
	}
	
	return *this;
}

}

#endif
