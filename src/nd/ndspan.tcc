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

#include <cassert>
#include "ndspan_iterator.h"


namespace mf {

template<std::size_t Dim, typename T>
bool ndspan<Dim, T>::invariant_() const {
	for(std::ptrdiff_t i = 0; i < Dim; ++i) if(end_[i] < start_[i]) return false;
	return true;
}



template<std::size_t Dim, typename T>
auto ndspan<Dim, T>::begin() const noexcept -> iterator {
	return iterator(*this, start_);
}


template<std::size_t Dim, typename T>
auto ndspan<Dim, T>::end() const noexcept -> iterator {
	coordinates_type iterator_end_coord = start_;
	iterator_end_coord.front() = end_.front();
	return iterator(*this, iterator_end_coord);
}



template<std::size_t Dim, typename T>
bool ndspan<Dim, T>::includes(const coordinates_type& c) const {
	for(std::ptrdiff_t i = 0; i < Dim; ++i)
		if( (start_[i] > c[i]) || (end_[i] <= c[i]) ) return false;
	return true;
}


template<std::size_t Dim, typename T>
bool ndspan<Dim, T>::includes(const ndspan& sub) const {
	for(std::ptrdiff_t i = 0; i < Dim; ++i)
		if( (start_[i] > sub.start_[i]) || (end_[i] < sub.end_[i]) ) return false;
	return true;
}


template<std::size_t Dim, typename T>
bool ndspan<Dim, T>::includes_strict(const ndspan& sub) const {
	for(std::ptrdiff_t i = 0; i < Dim; ++i)
		if( (start_[i] >= sub.start_[i]) || (end_[i] <= sub.end_[i]) ) return false;
	return true;
}


template<std::size_t Dim, typename T>
ndspan<Dim, T> span_intersection(const ndspan<Dim, T>& a, const ndspan<Dim, T>& b) {
	ndcoord<Dim, T> new_start, new_end;
	for(std::ptrdiff_t i = 0; i < Dim; ++i) {
		new_start[i] = std::max(a.start_pos()[i], b.start_pos()[i]);
		new_end[i] = std::min(a.end_pos()[i], b.end_pos()[i]);
		if(new_end[i] < new_start[i])
			new_start[i] = new_end[i] = 0;
	}
	return ndspan<Dim, T>(new_start, new_end);

}


}
