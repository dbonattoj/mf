#include <stdexcept>
#include <cstdlib>
#include <algorithm>

namespace mf {

namespace detail {
	template<std::size_t Dim, typename T, std::ptrdiff_t Target_dim>
	class ndarray_view_fcall : public ndarray_view<Dim, T> {
		static_assert(Target_dim <= Dim, "detail::ndarray_view_fcall target dimension out of bounds");
	
	public:
		using super = ndarray_view<Dim, T>;

		ndarray_view_fcall(const super& arr) :
			super(arr) { }
		
		ndarray_view_fcall<Dim, T, Target_dim + 1> operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const {
			return this->section_(Target_dim, start, end, step);
		}
		ndarray_view_fcall<Dim, T, Target_dim + 1> operator()(std::ptrdiff_t c) const {
			return this->section_(Target_dim, c, c + 1, 1);
		}
		ndarray_view_fcall<Dim, T, Target_dim + 1> operator()() const {
			return *this;
		}
	};
}


template<std::size_t Dim, typename T>
constexpr std::size_t ndarray_view<Dim, T>::dimension;

template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::default_strides(const shape_type& shape) -> strides_type {
	strides_type strides;
	strides[Dim - 1] = sizeof(T);
	for(std::ptrdiff_t i = Dim - 1; i > 0; --i)
		strides[i - 1] = strides[i] * shape[i];
	return strides;
}


template<std::size_t Dim, typename T>
ndarray_view<Dim, T>::ndarray_view(pointer start, const shape_type& shape) :
	ndarray_view(start, shape, default_strides(shape)) { }



template<std::size_t Dim, typename T>
ndarray_view<Dim, T>::ndarray_view(pointer start, const shape_type& shape, const strides_type& strides) :
	start_(start), shape_(shape), strides_(strides)
{
	std::ptrdiff_t i;
	contiguous_length_ = shape_.back();	
	for(i = Dim - 1; i > 0; i--) {
		if(strides_[i - 1] == shape_[i] * strides_[i]) contiguous_length_ *= shape_[i - 1];
		else break;
	}
}


template<std::size_t Dim, typename T>
std::ptrdiff_t ndarray_view<Dim, T>::fix_coordinate_(std::ptrdiff_t c, std::ptrdiff_t dimension) const {
	std::ptrdiff_t n = shape_[dimension];
	if(c < 0) c = n + c;
	if(c >= n) throw std::out_of_range("coordinate is out of range");
	return c;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::index_to_coordinates(const index_type& index) const -> coordinates_type {
	coordinates_type coord;
	index_type remainder = index;
	std::size_t factor = shape_.tail().product();
	for(std::ptrdiff_t i = 0; i < Dim - 1; ++i) {
		auto div = std::div(remainder, factor);
		coord[i] = div.quot;
		remainder = div.rem;
		factor /= shape_[i + 1];
	}
	coord.back() = remainder;
	return coord;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::coordinates_to_index(const coordinates_type& coord) const -> index_type {
	std::ptrdiff_t index = coord.back();
	std::ptrdiff_t factor = shape_[Dim - 1];
	for(std::ptrdiff_t i = Dim - 2; i >= 0; --i) {
		index += factor * coord[i];
		factor *= shape_[i];
	}
	return index;
}


template<std::size_t Dim, typename T>
void ndarray_view<Dim, T>::reset(const ndarray_view& other) noexcept {
	start_ = other.start_;
	shape_ = other.shape_;
	strides_ = other.strides_;
	contiguous_length_ = other.contiguous_length_;
}


template<std::size_t Dim, typename T>
void ndarray_view<Dim, T>::assign(const ndarray_view& other) const {
	if(shape() != other.shape()) throw std::invalid_argument("ndarray views must have same shape for assignment");
	std::copy(other.begin(), other.end(), begin());
}


template<std::size_t Dim, typename T>
bool ndarray_view<Dim, T>::compare(const ndarray_view& other) const {
	if(shape() != other.shape()) return false;
	else if(same(*this, other)) return true;
	else return std::equal(other.begin(), other.end(), begin());
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::coordinates_to_pointer(const coordinates_type& coord) const -> pointer {
	pointer ptr = start_;
	for(std::ptrdiff_t i = 0; i < Dim; ++i)
		ptr = advance_raw_ptr(ptr, strides_[i] * coord[i]);
	return ptr;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::at(const coordinates_type& coord) const -> reference {
	coordinates_type real_coord;
	for(std::ptrdiff_t i = 0; i < Dim; ++i) real_coord[i] = fix_coordinate_(coord[i], i);
	return *coordinates_to_pointer(real_coord);
}


template<std::size_t Dim, typename T>
inline auto ndarray_view<Dim, T>::begin() const -> iterator {
	return iterator(*this, 0, start_);
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::end() const -> iterator {
	index_type end_index = shape().product();
	coordinates_type end_coord = index_to_coordinates(end_index);
	return iterator(*this, end_index, coordinates_to_pointer(end_coord));
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::section_(std::ptrdiff_t i, std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const -> ndarray_view {
	using std::swap;

	if(start < 0) start = shape_[i] + start;
	if(end < 0) end = shape_[i] + end;
	
	if(start < 0 || start >= shape_[i]) throw std::out_of_range("section start out of range");
	if(end < 0 || end > shape_[i]) throw std::out_of_range("section end out of range");
	if(start >= end) throw std::invalid_argument("section start must be lower than end");
	if(step == 0) throw std::invalid_argument("section step must not be zero");	
	
	pointer new_start = advance_raw_ptr(start_, strides_[i] * start);
	shape_type new_shape = shape_;
	strides_type new_strides = strides_;
	
	std::ptrdiff_t n = end - start;
	new_strides[i] = strides_[i] * step;
	
	if(step > 0) {
		new_shape[i] = 1 + ((n - 1) / step);
	} else {
		new_shape[i] = 1 + ((n - 1) / -step);
		new_start = advance_raw_ptr(new_start, -new_strides[i] * (new_shape[i] - 1));
	}

	return ndarray_view(new_start, new_shape, new_strides);
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::section(const coordinates_type& start, const coordinates_type& end, const strides_type& steps) const -> ndarray_view {
	ndarray_view new_view = *this;
	for(std::ptrdiff_t i = 0; i < Dim; ++i)
		new_view.reset( new_view.section_(i, start[i], end[i], steps[i]) );
	return new_view;
}


template<std::size_t Dim, typename T>
auto ndarray_view<Dim, T>::slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const -> ndarray_view<Dim - 1, T> {
	return ndarray_view<Dim - 1, T>(
		advance_raw_ptr(start_, strides_[dimension] * fix_coordinate_(c, dimension)),
		shape_.erase(dimension),
		strides_.erase(dimension)
	);
}


}