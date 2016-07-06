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

namespace mf {

namespace detail {
	template<std::size_t Dim, typename T, std::ptrdiff_t Target_dim>
	class ndarray_timed_view_fcall : public ndarray_timed_view<Dim, T> {
		static_assert(Target_dim <= Dim, "detail::ndarray_timed_view_fcall target dimension out of bounds");
	
	public:
		using base = ndarray_timed_view<Dim, T>;

		ndarray_timed_view_fcall(const base& arr) :
			base(arr) { }
		
		ndarray_timed_view_fcall<Dim, T, Target_dim + 1>
		operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const {
			return this->section_(Target_dim, start, end, step);
		}
		ndarray_timed_view_fcall<Dim, T, Target_dim + 1>
		operator()(std::ptrdiff_t c) const {
			return this->section_(Target_dim, c, c + 1, 1);
		}
		ndarray_timed_view_fcall<Dim, T, Target_dim + 1>
		operator()() const {
			return *this;
		}
	};
}

}
