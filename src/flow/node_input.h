#ifndef MF_FLOW_NODE_INPUT_H_
#define MF_FLOW_NODE_INPUT_H_

#include <stdexcept>
#include "../common.h"
#include "../ndarray/ndarray_view.h"
#include "node_io_base.h"
#include "node_output.h"

namespace mf { namespace flow {

class node_base;

/// Input of a media node.
template<std::size_t Dim, typename T>
class node_input : public node_input_base {
public:
	using output_type = node_output<Dim, T>;
	
	using frame_view_type = ndarray_view<Dim, T>;
	using full_view_type = ndarray_view<Dim + 1, T>;
	using frame_shape_type = typename frame_view_type::shape_type;	

private:
	output_type* connected_output_ = nullptr; ///< Output of other media node to which input is connected.
	
	full_view_type view_;
	std::ptrdiff_t view_center_;

public:
	node_input(node_base& nd, time_unit past_window = 0, time_unit future_window = 0) :
		node_input_base(nd, past_window, future_window) { }
			
	void connect(output_type& output);
	bool is_connected() const noexcept { return (connected_output_ != nullptr); }
	output_type& connected_output() const override;
	
	const frame_shape_type& frame_shape() const { return connected_output().frame_shape(); }
		
	/// Begin reading next frame from input.
	/** Recursively pulls frame from connected node. View to the frame becomes available via view(), and to whole
	 ** window via full_view(). */
	void begin_read(time_unit t) override;
	
	/// Must be called after frame has been processed.
	/** Called after begin_read(). View is becomes longer available. reached_end() becomes true if this
	 ** was the last frame. */
	void end_read(time_unit t) override;
	
	/// Returns true when there is no more frame available.
	/** begin_read() cannot be called after this has returned true. */
	bool reached_end() const override;

	bool is_active() const { return connected_output_->is_active(); }
	
	/// Get readable view of current frame.
	/** Only callable while in media_node::process_(). */
	frame_view_type view() const { return view_[view_center_]; }
	
	const full_view_type& full_view() const { return view_; }	
	std::ptrdiff_t full_view_center() const { return view_center_; }
	full_view_type past_view() const { return view_(0, view_center_ + 1); }
	full_view_type future_view() const { return view_(view_center_, -1); }
};	

}}

#include "node_input.tcc"

#endif
