#ifndef MF_FLOW_NODE_FRAME_VIEW_H_
#define MF_FLOW_NODE_FRAME_VIEW_H_

#include "../../queue/frame.h"
#include "../node_frame_satellite.h"
#include "../node_frame_format.h"

namespace mf { namespace flow {

/// Frame view for frames passing through nodes.
/** Convenience wrapper for \ref mf::frame_view (i.e. `ndarray_opaque_view`) with format \ref node_frame_format.
 ** Provides direct access to frame satellite data. */
class node_frame_view : public frame_view {
public:
	using base = frame_view;

	using base::base;

	const node_frame_format& frame_format() {
		return static_cast<const node_frame_format&>(base::frame_format());
	}

	node_frame_satellite& satellite() {
		return frame_format().frame_satellite(start());
	}
	
	node_parameter_valuation& propagated_parameters() {
		return satellite().propagated_parameters();
	}
};

}}

#endif
