#ifndef MF_NDARRAY_TIMED_RING_H_
#define MF_NDARRAY_TIMED_RING_H_

#include <atomic>
#include "ndarray_ring.h"

namespace mf {

/// Ndarray ring which keeps track of absolute time.
template<std::size_t Dim, typename T>
class ndarray_timed_ring : public ndarray_ring<Dim, T> {
	using base = ndarray_ring<Dim, T>;

private:
	std::atomic<time_unit> last_write_time_; ///< Time of last written frame.

public:
	using typename base::section_view_type;

	ndarray_timed_ring(const ndsize<Dim>& frames_shape, std::size_t duration, time_unit time_offset = 0) :
		base(frames_shape, duration), last_write_time_(time_offset - 1) { }
	
	time_unit current_time() const noexcept { return last_write_time_; }
	
	void end_write(std::size_t written_duration) override;
	
	time_span readable_time_span() const;
	time_span writable_time_span() const;
	
	section_view_type begin_write_span(time_span);
	section_view_type begin_read_span(time_span);
	void skip_span(time_span);
};


}

#include "ndarray_timed_ring.tcc"

#endif
