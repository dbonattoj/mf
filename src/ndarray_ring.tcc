#include <stdexcept>
#include <cmath>
#include "util.h"

namespace mf {

template<std::size_t Dim, typename T>
ndarray_ring<Dim, T>::ndarray_ring(const ndsize<Dim>& frame_shape, std::size_t duration) :
	base(	
		ndcoord_cat(ndsize<1>(duration), frame_shape),
		adjust_padding_(frame_shape, duration),
		ring_allocator<T>()
	) { }


template<std::size_t Dim, typename T>
auto ndarray_ring<Dim, T>::adjust_padding_(const ndsize<Dim>& frame_shape, std::size_t duration) -> padding_type {
	std::size_t frame_size = frame_shape.product();
	std::size_t page_size = ring_allocator<T>::page_size();
	
	std::size_t frame_padding = 0;
	while( (duration * (frame_size + frame_padding)) % page_size != 0 ) ++frame_padding;
		
	padding_type new_padding(0);
	new_padding.front() = frame_padding;	
		
	return new_padding;
}


template<std::size_t Dim, typename T>
auto ndarray_ring<Dim, T>::section_(std::ptrdiff_t start, std::size_t duration) -> section_view_type {
	if(duration > base::shape_.front()) throw std::invalid_argument("ring section duration too large");
	
	auto new_start = advance_raw_ptr(base::start_, base::strides_.front() * start);
	auto new_shape = base::shape_;
	new_shape[0] = duration;
	auto new_strides = base::strides_;
	
	return section_view_type(new_start, new_shape, new_strides);
}


template<std::size_t Dim, typename T>
std::size_t ndarray_ring<Dim, T>::readable_duration() const noexcept {
	if(full_) return total_duration();
	else if(read_position_ <= write_position_) return write_position_ - read_position_;
	else return total_duration() - read_position_ + write_position_;
}


template<std::size_t Dim, typename T>
std::size_t ndarray_ring<Dim, T>::writable_duration() const noexcept {
	return total_duration() - readable_duration();
}


template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::write(std::size_t duration, const std::function<write_function>& fct) {
	if(duration > writable_duration()) throw std::invalid_argument("write duration too large");
	
	auto sec = section_(write_position_, duration);
	fct(sec);
	
	write_position_ = (write_position_ + duration) % total_duration();
	if(write_position_ == read_position_) full_ = true;
}


template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::read(std::size_t duration, const std::function<read_function>& fct) {
	if(duration > readable_duration()) throw std::invalid_argument("read duration too large");

	auto sec = section_(read_position_, duration);
	fct(sec);

	read_position_ = (read_position_ + duration) % total_duration();
	full_ = false;
}


}