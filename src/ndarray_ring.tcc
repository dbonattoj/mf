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
	if(duration > base::shape().front()) throw std::invalid_argument("ring section duration too large");
	
	auto new_start = advance_raw_ptr(base::start(), base::strides().front() * start);
	auto new_shape = base::shape();
	new_shape[0] = duration;
	auto new_strides = base::strides();
	
	return section_view_type(new_start, new_shape, new_strides);
}


template<std::size_t Dim, typename T>
std::size_t ndarray_ring<Dim, T>::readable_duration() const {
	if(full_) return total_duration();
	else if(read_position_ <= write_position_) return write_position_ - read_position_;
	else return total_duration() - read_position_ + write_position_;
}


template<std::size_t Dim, typename T>
std::size_t ndarray_ring<Dim, T>::writable_duration() const {
	return total_duration() - readable_duration();
}


template<std::size_t Dim, typename T>
auto ndarray_ring<Dim, T>::begin_write(std::size_t duration) -> section_view_type {
	if(duration > total_duration()) throw std::invalid_argument("write duration larger than ring capacity");
	else if(duration > writable_duration()) throw sequencing_error("write duration larger than writable frames");
	else return section_(write_position_, duration);
}


template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::end_write(std::size_t written_duration) {
	if(written_duration > writable_duration()) throw std::invalid_argument("reported written duration too large");
	write_position_ = (write_position_ + written_duration) % total_duration();
	if(write_position_ == read_position_) full_ = true;
}
	

template<std::size_t Dim, typename T>
auto ndarray_ring<Dim, T>::begin_read(std::size_t duration) -> section_view_type {
	if(duration > total_duration()) throw std::invalid_argument("read duration larger than ring capacity");
	else if(duration > readable_duration()) throw sequencing_error("read duration larger than readable frames");
	else return section_(read_position_, duration);
}


template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::end_read(std::size_t read_duration) {
	if(read_duration > readable_duration()) throw std::invalid_argument("reported read duration too large");
	read_position_ = (read_position_ + read_duration) % total_duration();
	full_ = false;
}


template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::skip(std::size_t duration) {
	// no check whether duration > total_duration: unlike begin_read/write, the data to skip will not be accessed
	// from a view, and subclass (shared) can skip more than the buffer capacity
	if(duration > readable_duration()) throw sequencing_error("skip duration larger than readable frames");
	read_position_ = (read_position_ + duration) % total_duration();
	full_ = false;
}


}