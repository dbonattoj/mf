#ifndef MF_NDARRAY_TIMED_RING_H_
#define MF_NDARRAY_TIMED_RING_H_

#include <atomic>
#include <ostream>
#include "ndarray_ring.h"

namespace mf {

/// Ndarray ring which keeps track of absolute time.
template<std::size_t Dim, typename T>
class ndarray_timed_ring : public ndarray_ring<Dim, T> {
	using base = ndarray_ring<Dim, T>;

private:
	/// Time of last written frame.
	/** Defined _current time_ of timed ring buffer. Next write starts at `last_write_time_ + 1`. */
	std::atomic<time_unit> last_write_time_{-1};

public:
	using typename base::section_view_type;

	ndarray_timed_ring(const ndsize<Dim>& frames_shape, std::size_t duration) :
		base(frames_shape, duration) { }
	
	void initialize() override;
	
	time_unit current_time() const noexcept { return last_write_time_; }
	time_unit read_start_time() const noexcept;
	time_unit write_start_time() const noexcept;
	
	void end_write(std::size_t written_duration) override;
		
	time_span readable_time_span() const;
	time_span writable_time_span() const;
	
	bool can_write_span(time_span) const;
	bool can_read_span(time_span) const;
	bool can_skip_span(time_span) const;
	
	section_view_type begin_write_span(time_span);
	section_view_type begin_read_span(time_span);
	void skip_span(time_span);
	
	void seek(time_unit);
	
	#ifndef NDEBUG
	void debug_print(std::ostream&) const;
	#endif
};


}

#include "ndarray_timed_ring.tcc"

#endif
