#ifndef MF_UTILITY_STOPWATCH_H_
#define MF_UTILITY_STOPWATCH_H_

#include "../common.h"
#include <chrono>

namespace mf {

/// Stop watch which measures total time passed between start() and stop() calls.
/** Can accumulate multiple time intervals. */
class stopwatch {
public:
	using clock_type = std::chrono::high_resolution_clock;
	using time_point_type = typename clock_type::time_point;
	using duration_type = typename clock_type::duration;

private:
	duration_type total_duration_ = duration_type::zero();
	time_point_type start_time_;
	bool running_ = false;

public:
	/// Reset accumulated duration and stop.
	void reset() {
		total_duration_ = duration_type::zero();
		running_ = false;
	}
	
	/// Start the stopwatch.
	void start() {
		start_time_ = clock_type::now();
		running_ = true;
	}
	
	/// Stop the stopwatch, and add duration since last `start()` call to total duration.
	void stop() {
		if(! running_) return;
		running_ = false;
		time_point_type end_time = clock_type::now();
		duration_type duration = end_time - start_time_;
		total_duration_ += duration;
		start_time_ = end_time;
	}

	/// Get total duration.
	/** Cannot be called while stopwatch is running. */
	duration_type total_duration() const {
		MF_ASSERT(! running_);
		return total_duration_;
	}
};

}

#endif
