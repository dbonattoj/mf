#include <algorithm>

#include <iostream>

namespace mf {


template<std::size_t Dim, typename T>
auto ndarray_shared_ring<Dim, T>::begin_write(std::size_t duration) -> section_view_type {
	// duration test, before waiting
	if(duration > base::total_duration()) throw std::invalid_argument("write duration larger than ring capacity");

	// locking positions mutex
	std::unique_lock<std::mutex> lock(positions_mutex_);

	// wait on it, until writable duration is sufficient
	// each time frame(s) are read/skipped writable_cv_ gets notified
	// robust against spurious wake-ups, does not start waiting if enough frames were already writable
	while(base::writable_duration() < duration) writable_cv_.wait(lock);

	// now return view to writable frames
	// base::begin_write() not thread-safe, but positions_mutex_ is still locked until return 
	return base::begin_write(duration);
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::end_write(std::size_t written_duration) {
	{
		// while mutex locked, update positions in base class
		std::lock_guard<std::mutex> lock(positions_mutex_);
		base::end_write(written_duration);
	}
	// notify condition variable: more readable frames have become available
	readable_cv_.notify_one();
}


template<std::size_t Dim, typename T>
auto ndarray_shared_ring<Dim, T>::begin_read(std::size_t duration) -> section_view_type {
	if(duration > base::total_duration()) throw std::invalid_argument("read duration larger than ring capacity");
	std::unique_lock<std::mutex> lock(positions_mutex_);
	while(base::readable_duration() < duration) readable_cv_.wait(lock);
	return base::begin_read(duration);
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::end_read(std::size_t read_duration) {
	{
		std::lock_guard<std::mutex> lock(positions_mutex_);
		base::end_read(read_duration);
	}
	writable_cv_.notify_one();
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::skip_available_(std::size_t duration) {
	// can only skip frames that are already readable
	assert(duration <= this->readable_duration());
	
	{
		// while mutex locked, update positions in base class
		std::lock_guard<std::mutex> lock(positions_mutex_);
		base::skip(duration);
	}
	
	// notify condition variable: more writable frames have become available
	writable_cv_.notify_one();
}


template<std::size_t Dim, typename T>
void ndarray_shared_ring<Dim, T>::skip(std::size_t skip_duration) {
	// duration may be larger than ring capacity
	
	const auto initial_readable_duration = this->readable_duration();
	const auto total_duration = base::total_duration();
	
	if(skip_duration <= initial_readable_duration) {
		// simple case: only skip some of the already readable frames
		skip_available_(skip_duration);

	} else {
		auto remaining_skip_duration = skip_duration;
			
		// wait for whole buffer to fill up and then skip all frames
		// until less than buffer size remains to be skipped
		while(remaining_skip_duration >= total_duration) {
			begin_read(total_duration); // ...wait for buffer to fill up entirely
			skip_available_(total_duration); // ...skip all and notify writable_cv_
			remaining_skip_duration -= total_duration;
		}
		
		// now skip remaining frames, waiting for them if necessary
		begin_read(remaining_skip_duration);
		skip_available_(remaining_skip_duration);
		
		// there may be new readable frames already available (after skipped frames),
		// if >1 frames got written at a time
	}
}


}