#ifndef MF_NDARRAY_SHARED_RING_H_
#define MF_NDARRAY_SHARED_RING_H_

#include <mutex>
#include <condition_variable>
#include "ndarray_ring.h"

namespace mf {

template<std::size_t Dim, typename T>
class ndarray_shared_ring : private ndarray_ring<Dim, T> {
	using base = ndarray_ring<Dim, T>;

private:
	std::mutex read_mutex_; ///< Mutex which gets locked while reading.
	std::mutex write_mutex_;
	
	std::condition_variable readable_cv_; ///< Condition variable which gets notified when readable frames become available.
	std::condition_variable writable_cv_; ///< Condition variable which gets notified when writable frames become available.

public:
	using typename base::section_view_type;

	class handle : public section_view_type {
	protected:
		ndarray_shared_ring& ring_;
		std::unique_lock<std::mutex> lock_;
		read_handle(ndarray_shared_ring& ring, std::mutex& mut, const section_view_type& sec) :
			ring_(ring), lock_(mut) { }
	public:
		void release() { lock_.unlock(); }
	};
	
	class read_handle : public handle {
	public:
		using handle::handle;
		void did_read(std::size_t n) {
			if(n > handle::shape()[0]) throw std::invalid_argument("reported more read than requested");
			else ring_.did_read(n);
		}
	};
	
	class write_handle : public handle {
	public:
		using handle::handle;
		void did_write(std::size_t n) {
			if(n > handle::shape()[0]) throw std::invalid_argument("reported more written than requested");
			else ring_.did_write(n);
		}
	};


	read_handle read(std::size_t duration) {
		auto sec = base::read(duration);
		return read_handle(*this, read_mutex_, sec);
	}
	
	write_handle write(std::size_t duration) {
		auto sec = base::write(duration);
		return write_handle(*this, write_mutex_, sec);	
	}
	
	void skip(std::size_t duration) {
		std::lock_guard<std::mutex> lock_(read_lock_);
		base::skip(duration);
	}
};

}

#endif
