#ifndef MF_NDARRAY_SHARED_RING_H_
#define MF_NDARRAY_SHARED_RING_H_

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <atomic>
#include "ndarray_timed_ring.h"

namespace mf {

/// Ndarray timed ring for concurrent read/write access.
/** For use with two threads: one which reads and one which writes. Does not support having multiple parallel reads
 ** or multiple parallel writes. Enhancements:
 ** - Mutex lock on buffer state (read and write positions)
 ** - Semantics of begin_write(), begin_read() and skip() changed to wait until frames become available 
 ** - Writer can mark end of file. begin_read(), begin_read_span() and skip() then to not wait for more frames
 ** Base class functions which depend on the read/write positions (e.g. readable_duration(), writable_time_span(),
 ** etc) are not thread-safe. Functions shared_readable_duration() are, and also consider the marked end. **/
template<std::size_t Dim, typename T>
class ndarray_shared_ring : public ndarray_timed_ring<Dim, T> {
	using base = ndarray_timed_ring<Dim, T>;

private:
	enum thread_state { idle, waiting, accessing };

	mutable std::mutex positions_mutex_; ///< Mutex which gets locked while advancing pointers.
	std::condition_variable writable_cv_; ///< Condition variable, gets notified when writable frames become available.
	std::condition_variable readable_cv_; ///< Condition variable, gets notified when readable frames become available.

	std::atomic<thread_state> reader_state_{idle};
	std::atomic<thread_state> writer_state_{idle};

	bool read_reaches_eof_ = false;
	std::atomic<time_unit> end_time_{-1};
	
	void skip_available_(std::size_t duration);
	
public:
	using typename base::section_view_type;

	ndarray_shared_ring(const ndsize<Dim>& frames_shape, std::size_t duration) :
		base(frames_shape, duration) { }
	
	/// Reinitialize ring buffer to state on construction.
	void initialize() override;
	
	section_view_type begin_write(std::size_t duration) override;
	void end_write(std::size_t written_duration) override;
	void end_write(std::size_t written_duration, bool eof);

	section_view_type begin_read(std::size_t read_duration) override;
	bool read_reaches_eof() const { return read_reaches_eof_; }
	void end_read(std::size_t read_duration) override;	
	
	void skip(std::size_t skip_duration) override;
	
	void lock() { positions_mutex_.lock(); }
	bool try_lock() { return positions_mutex_.try_lock(); }
	void unlock() { positions_mutex_.unlock(); }
	
	time_unit shared_readable_duration() const;
	time_span shared_readable_time_span() const;
	time_unit shared_writable_duration() const;
	time_span shared_writable_time_span() const;
	
	/// Returns true if the EOF was marked by the writer.
	/** EOF is marked by writer by an end_write() with \a eof argument set. */
	bool eof_was_marked() const;
};

}

#include "ndarray_shared_ring.tcc"

#endif
