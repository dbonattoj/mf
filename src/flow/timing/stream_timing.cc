#include "stream_timing.h"

namespace mf { namespace flow {
	
stream_timing stream_timing::real_time() {
	stream_timing timing;
	timing.set_real_time(true);
	return timing;
}

bool operator==(const stream_timing& a, const stream_timing& b) {
	if(a.real_time_ && b.real_time_) return true;
	else if(a.real_time_ != b.real_time_) return false;
	else return (a.frame_clock_duration_ == b.frame_clock_duration_);
}


bool operator!=(const stream_timing& a, const stream_timing& b) {
	return !(a == b);
}

}}
