#include <stdexcept>
#include <cmath>
#include "../utility/misc.h"
#include "../utility/memory.h"

namespace mf {
	

template<std::size_t Dim, typename T>
ndarray_ring<Dim, T>::ndarray_ring(const ndsize<Dim>& frame_shape, time_unit duration) :
	base(	
		ndcoord_cat(ndsize<1>(duration), frame_shape),
		adjust_padding_(frame_shape, duration),
		raw_ring_allocator()
	) { }


template<std::size_t Dim, typename T>
std::size_t ndarray_ring<Dim, T>::adjust_padding_(const ndsize<Dim>& frame_shape, std::size_t duration) {
	std::size_t frame_size = frame_shape.product() * sizeof(T);
	std::size_t page_size = system_page_size();

	constexpr std::size_t a = alignof(T);

	// will compute minimal padding frame_padding (bytes to insert between frames)
	// such that duration * (frame_size + frame_padding) is a multiple of page_size
	//       and (frame_size + frame_padding) is a multiple of a
	
	// frame_size and page_size are necessarily multiples of a, because:
	//    a and page_size are both powers of 2,
	//    page_size >>> a,
	//    and sizeof(T) is always multiple of a
	// need to make frame_padding also multiple of a
	// --> count in units a
	
	static_assert(sizeof(T) % a == 0, "sizeof(T) is not a multiple of alignof(T)");
	
	MF_ASSERT(frame_size % a == 0, "frame size not multiple of alignof(T)");
	MF_ASSERT(page_size % a == 0, "system page size not multiple of alignof(T)");
	
	std::size_t frame_size_a = frame_size / a;
	std::size_t page_size_a = page_size / a;
	// duration * (frame_size_a + frame_padding_a) must be multiple of page_size_a
	
	std::size_t d = page_size_a / gcd(page_size_a, duration);	
	// --> (frame_size_a + frame_padding_a) must be multiple of d
	// d is a power of 2
	// d = factors 2 that are missing in duration for it to be multiple of page_size_a
	
	// if frame_size_a is already multiple of d, no padding is needed
	std::size_t r = frame_size_a % d;
	if(r == 0) return 0;
	else return (d - r) * a;
}


template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::initialize() {
	read_position_ = 0;
	write_position_ = 0;
	full_ = false;
}


template<std::size_t Dim, typename T>
auto ndarray_ring<Dim, T>::section_(time_unit start, time_unit duration) -> section_view_type {
	if(duration > base::shape().front()) throw std::invalid_argument("ring section duration too large");
	
	auto new_start = advance_raw_ptr(base::start(), base::strides().front() * start);
	auto new_shape = base::shape();
	new_shape[0] = duration;
	auto new_strides = base::strides();
	
	return section_view_type(new_start, new_shape, new_strides);
}


template<std::size_t Dim, typename T>
time_unit ndarray_ring<Dim, T>::readable_duration() const {
	if(full_) return total_duration();
	else if(read_position_ <= write_position_) return write_position_ - read_position_;
	else return total_duration() - read_position_ + write_position_;
}


template<std::size_t Dim, typename T>
time_unit ndarray_ring<Dim, T>::writable_duration() const {
	return total_duration() - readable_duration();
}


template<std::size_t Dim, typename T>
auto ndarray_ring<Dim, T>::begin_write(time_unit duration) -> section_view_type {
	if(duration > total_duration()) throw std::invalid_argument("write duration larger than ring capacity");
	else if(duration > writable_duration()) throw sequencing_error("write duration larger than writable frames");
	else return section_(write_position_, duration);
}


template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::end_write(time_unit written_duration) {
	if(written_duration == 0) return;
	if(written_duration > writable_duration()) throw std::invalid_argument("reported written duration too large");
	write_position_ = (write_position_ + written_duration) % total_duration();
	if(write_position_ == read_position_) full_ = true;
}
	

template<std::size_t Dim, typename T>
auto ndarray_ring<Dim, T>::begin_read(time_unit duration) -> section_view_type {
	section_view_type res;
	if(duration > total_duration()) throw std::invalid_argument("read duration larger than ring capacity");
	else if(duration > readable_duration()) throw sequencing_error("read duration larger than readable frames");
	else return section_(read_position_, duration);
}


template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::end_read(time_unit read_duration) {
	if(read_duration == 0) return;
	if(read_duration > readable_duration()) throw std::invalid_argument("reported read duration too large");
	read_position_ = (read_position_ + read_duration) % total_duration();
	full_ = false;
}


template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::skip(time_unit duration) {
	// no check whether duration > total_duration: unlike begin_read/write, the data to skip will not be accessed
	if(duration == 0) return;
	if(duration > readable_duration()) throw std::invalid_argument("skip duration larger than readable frames");
	read_position_ = (read_position_ + duration) % total_duration();
	full_ = false;
}


#ifndef NDEBUG
template<std::size_t Dim, typename T>
void ndarray_ring<Dim, T>::debug_print(std::ostream& str) const {
	str << "ndarray_ring: "
	    << "read_position=" << read_position_ << ", write_position=" << write_position_ << ", full=" << full_
	    << "\ntotal_duration=" << total_duration() << std::endl;
	 
}
#endif


}
