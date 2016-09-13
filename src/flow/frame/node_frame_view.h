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

#ifndef MF_FLOW_NODE_FRAME_VIEW_H_
#define MF_FLOW_NODE_FRAME_VIEW_H_

#include "../../queue/frame.h"
#include "node_frame_satellite.h"
#include "node_frame_format.h"

namespace mf { namespace flow {

/// View for frame accessed by node.
/** Convenience wrapper for \ref mf::frame_view (i.e. `ndarray_opaque_view`) with format \ref node_frame_format.
 ** Provides direct access to frame satellite data. */
class node_frame_view : public frame_view {
	using base = frame_view;
	
public:
	node_frame_view() = default;
	node_frame_view(const node_frame_view&) = default;

	node_frame_view(const base& vw) : base(vw) { }

	const node_frame_format& frame_format() const {
		return static_cast<const node_frame_format&>(base::frame_format());
	}

	node_frame_satellite& satellite() const {
		return frame_format().frame_satellite(start());
	}
	
	node_parameter_valuation& propagated_parameters() const {
		return satellite().propagated_parameters();
	}
};


/// View for timed frame array accessed by node.
/** Convenience wrapper for \ref mf::timed_frame_array_view with format \ref node_frame_format.
 ** Provides access to singular frames as \ref node_frame_view, and stores index of _center_ frame (for _current time
 ** index_ of node). */
class node_frame_window_view : public timed_frame_array_view {
	using base = timed_frame_array_view;
	
private:
	std::ptrdiff_t center_index_;

public:
	node_frame_window_view() = default;
	node_frame_window_view(const node_frame_window_view&) = default;

	node_frame_window_view(const base& vw, time_unit center_time) :
		base(vw),
		center_index_(center_time - vw.start_time()) { }
		
	const node_frame_format& frame_format() const {
		return static_cast<const node_frame_format&>(base::frame_format());
	}
	
	node_frame_view operator[](std::ptrdiff_t index) const {
		return node_frame_view(base::operator[](index));
	}
	
	node_frame_view at_time(time_unit t) const {
		return operator[](t - start_time());
	}
	
	node_frame_view now() const {
		return operator[](center_index_);
	}
};

}}

#endif
