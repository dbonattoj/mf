#ifndef MF_FLOW_NODE_OUTPUT_H_
#define MF_FLOW_NODE_OUTPUT_H_

#include <memory>
#include <stdexcept>
#include "../common.h"
#include "../ndarray/ndarray_view.h"
#include "../ndarray/ndarray_shared_ring.h"
#include "node_io_base.h"

namespace mf { namespace flow {

class node_base;

/// Output of a media node.
template<std::size_t Dim, typename T>
class node_output : public node_output_base {
public:
	using frame_view_type = ndarray_view<Dim, T>;
	using frame_shape_type = typename frame_view_type::shape_type;

	using ring_type = ndarray_shared_ring<Dim, T>;

public:
	ndsize<Dim> frame_shape_;

	std::unique_ptr<ring_type> buffer_; ///< Ring buffer containing the output frames.
	bool view_available_ = false;
	frame_view_type view_;
	
public:	
	explicit node_output(node_base& nd) :
		node_output_base(nd) { }
		
	void define_frame_shape(const frame_shape_type& shp);
	bool frame_shape_is_defined() const override { return frame_shape_.product() != 0; }
	const frame_shape_type& frame_shape() const;
	
	/// Set up the node output.
	/** Frame shape and required buffer duration, and stream duration (if seekable) must have been defined.
	 ** Creates the ring buffer. */
	void setup() override;
		
	ring_type& ring() { return *buffer_; }
	
	frame_view_type& view() { MF_ASSERT(view_available_, "view not available"); return view_; }
	
	bool view_is_available() const { return view_available_; }
		
	time_unit begin_write() override {
		MF_DEBUG("output::begin_write()....");
		auto view = ring().begin_write(1);
		if(view.duration() != 1) throw std::runtime_error("output at end");
		view_.reset(view[0]);
		MF_DEBUG("output::begin_write() --> ", view.span(), " t=", view.start_time());
		view_available_ = true;
		return view.start_time();
	}
	
	void didnt_write() override {
		ring().end_write(0);
	}
	
	void end_write(bool is_last_frame) override {
		MF_DEBUG("output::end_write()....");
		if(ring().is_seekable()) {
			ring().end_write(1);
			if(is_last_frame) assert(ring().writer_reached_end());
		} else {
			ring().end_write(1, is_last_frame);
		}
		MF_DEBUG("output::end_write()");
		view_available_ = false;
	}


	#ifndef NDEBUG
	void debug_print(std::ostream&) const override;
	#endif
};

}}

#include "node_output.tcc"

#endif
