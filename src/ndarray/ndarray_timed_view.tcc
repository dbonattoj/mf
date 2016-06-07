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
