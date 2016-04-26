#include "ring.h"
#include <stdexcept>
#include <cmath>
#include "../utility/misc.h"
#include "../utility/memory.h"

namespace mf {
	

ring::ring(const frame_properties& frame_prop, time_unit duration) :
	base(frame_prop, duration, adjust_padding_(frame_prop, duration), raw_ring_allocator()) { }


std::size_t ring::adjust_padding_(const frame_properties& frame_prop, std::size_t duration) {
	std::size_t frame_size = frame_prop.length;
	std::size_t page_size = system_page_size();

	std::size_t a = frame_prop.alignment;

	// will compute minimal padding frame_padding (bytes to insert between frames)
	// such that duration * (frame_size + frame_padding) is a multiple of page_size
	//       and (frame_size + frame_padding) is a multiple of a
	
	// frame_size and page_size are necessarily multiples of a, because:
	//    a and page_size are both powers of 2,
	//    page_size >>> a,
	//    frame_size is required to be multiple of a
	// need to make frame_padding also multiple of a
	// --> count in units a
		
	MF_ASSERT_MSG(frame_size % a == 0, "frame size not multiple of frame alignment");
	MF_ASSERT_MSG(page_size % a == 0, "system page size not multiple of frame alignment");
	
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


void ring::initialize() {
	read_position_ = 0;
	write_position_ = 0;
	full_ = false;
}


auto ring::section_(time_unit start, time_unit duration) -> section_view_type {
	if(duration > shape().front()) throw std::invalid_argument("ring section duration too large");
	
	auto new_start = base::start() + (strides().front() * start);
	auto new_shape = shape();
	new_shape[0] = duration;
	auto new_strides = strides();
	
	return section_view_type(new_start, new_shape, new_strides);
}


time_unit ring::readable_duration() const {
	if(full_) return total_duration();
	else if(read_position_ <= write_position_) return write_position_ - read_position_;
	else return total_duration() - read_position_ + write_position_;
}


time_unit ring::writable_duration() const {
	return total_duration() - readable_duration();
}


auto ring::begin_write(time_unit duration) -> section_view_type {
	if(duration > total_duration()) throw std::invalid_argument("write duration larger than ring capacity");
	else if(duration > writable_duration()) throw sequencing_error("write duration larger than writable frames");
	else return section_(write_position_, duration);
}


void ring::end_write(time_unit written_duration) {
	if(written_duration == 0) return;
	if(written_duration > writable_duration()) throw std::invalid_argument("reported written duration too large");
	write_position_ = (write_position_ + written_duration) % total_duration();
	if(write_position_ == read_position_) full_ = true;
}
	

auto ring::begin_read(time_unit duration) -> section_view_type {
	section_view_type res;
	if(duration > total_duration()) throw std::invalid_argument("read duration larger than ring capacity");
	else if(duration > readable_duration()) throw sequencing_error("read duration larger than readable frames");
	else return section_(read_position_, duration);
}


void ring::end_read(time_unit read_duration) {
	if(read_duration == 0) return;
	if(read_duration > readable_duration()) throw std::invalid_argument("reported read duration too large");
	read_position_ = (read_position_ + read_duration) % total_duration();
	full_ = false;
}


void ring::skip(time_unit duration) {
	// no check whether duration > total_duration: unlike begin_read/write, the data to skip will not be accessed
	if(duration == 0) return;
	if(duration > readable_duration()) throw std::invalid_argument("skip duration larger than readable frames");
	read_position_ = (read_position_ + duration) % total_duration();
	full_ = false;
}


}
