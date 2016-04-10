#ifndef MF_NDARRAY_TIMED_RING_H_
#define MF_NDARRAY_TIMED_RING_H_

#include <atomic>
#include <ostream>
#include "ndarray_ring.h"
#include "ndarray_view.h"

namespace mf {

/// Ndarray view with absolute time indices associated to first dimension.
template<std::size_t Dim, typename T>
class ndarray_timed_view : public ndarray_view<Dim, T> {
	using base = ndarray_view<Dim, T>;

private:
	time_unit start_time_;
	
public:
	ndarray_timed_view(const base& vw, time_unit start_time) :
		base(vw), start_time_(start_time) { }

	time_unit start_time() const {
		return start_time_;
	}
	time_unit end_time() const {
		return start_time_ + base::shape().front();
	}
	time_unit time_at(std::ptrdiff_t i) const {
		return start_time_ + base::fix_coordinate_(i, 0);
	}
};



/// Ndarray ring which keeps track of absolute time.
template<std::size_t Dim, typename T>
class ndarray_timed_ring : public ndarray_ring<Dim, T> {
	using base = ndarray_ring<Dim, T>;

private:
	/// Time of last written frame.
	/** Defined _current time_ of timed ring buffer. Next write starts at `last_write_time_ + 1`. */
	std::atomic<time_unit> last_write_time_{-1};
	// TODO remove atomic

public:
	using section_view_type = ndarray_timed_view<Dim + 1, T>;

	ndarray_timed_ring(const ndsize<Dim>& frames_shape, time_unit duration) :
		base(frames_shape, duration) { }
	
	void initialize();
	
	time_unit current_time() const noexcept { return last_write_time_; }
	time_unit read_start_time() const noexcept;
	time_unit write_start_time() const noexcept;
	
	time_span readable_time_span() const;
	time_span writable_time_span() const;
	
	bool can_write_span(time_span) const;
	bool can_read_span(time_span) const;
	bool can_skip_span(time_span) const;
	
	section_view_type begin_write(time_unit);
	section_view_type begin_write_span(time_span);
	void end_write(time_unit written_duration);
	
	section_view_type begin_read(time_unit);
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
