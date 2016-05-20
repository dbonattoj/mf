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

namespace mf {

template<std::size_t Dim, typename T>
ndspan<Dim, T>::ndspan(const coordinates_type& start, const coordinates_type& end):
	start_(start), end_(end)
{
	for(std::ptrdiff_t i = 0; i < Dim; ++i) assert(end_[i] >= start_[i]);
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
	}
	return ndspan<Dim, T>(new_start, new_end);

}


}
