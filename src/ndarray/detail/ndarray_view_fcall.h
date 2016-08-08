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

#ifndef MF_NDARRAY_VIEW_FCALL_H_
#define MF_NDARRAY_VIEW_FCALL_H_

#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <type_traits>

namespace mf { namespace detail {

template<typename View, std::ptrdiff_t Target_dim>
class ndarray_view_fcall : public View {
	static_assert(Target_dim <= View::dimension, "detail::ndarray_view_fcall target dimension out of bounds");
	using base = View;

private:
	using fcall_type = ndarray_view_fcall<View, Target_dim + 1>;

public:
	using base::base;
	
	ndarray_view_fcall(const base& vw) : base(vw) { }
	
	fcall_type operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const {
		return base::section_(Target_dim, start, end, step);
	}
	fcall_type operator()(std::ptrdiff_t c) const {
		return base::section_(Target_dim, c, c + 1, 1);
	}
	fcall_type operator()() const {
		return *this;
	}		
};

}}

#endif
