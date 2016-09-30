/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_FLOW_NODE_STREAM_TIMING_H_
#define MF_FLOW_NODE_STREAM_TIMING_H_

#include "../../common.h"
#include "../types.h"
#include <stdexcept>

namespace mf { namespace flow {

/// Timing of stream flowing through a node.
/** Defines the meaning of the frame indices. The "current time" `t` of a node is a discrete frame index, where as the
 ** "clock time" `ct` is a real time value, relative to some start clock time. The stream timing maps the frame
 ** indices of the node to clock times. Two policies exist, _realtime_ and _non-realtime_.
 ** For _realtime_, the clock time of each frame corresponds to the current system clock time at the moment when the
 ** frame is processed (minus the start clock time).
 ** For _non-realtime_, it is computed using as `ct = offset + frame_clock_duration*t`.
 ** The graph-wide start clock time is set to the current system clock time when the graph is launched. */
class node_stream_timing {
private:
	bool real_time_ = false;
	clock_duration frame_clock_duration_ = clock_duration();
	//time_unit offset_ = 0;

public:	
	bool is_real_time() const { return real_time_; }
	void set_real_time(bool rt) { real_time_ = rt; }
	
	bool has_frame_clock_duration() const { return !real_time_ && (frame_clock_duration_ != clock_duration()); }
	clock_duration frame_clock_duration() const { Assert(!real_time_); return frame_clock_duration_; }
	void set_frame_clock_duration(const clock_duration& dur) { frame_clock_duration_ = dur; } 
	void set_no_frame_clock_duration(const clock_duration& dur) { frame_clock_duration_ = clock_duration(); } 
	
	//time_unit offset() const { Assert(!real_time_); return offset_; }
	//void set_offset(time_unit off) { Assert(!real_time_); offset_ = off; }
	// TODO offset
};

/*
policies:
- animation -->  t |--> clocktime
- realtime
- still
*/

}}

#endif
