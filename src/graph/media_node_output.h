#ifndef MF_MEDIA_NODE_OUTPUT_H_
#define MF_MEDIA_NODE_OUTPUT_H_

#include <memory>
#include <stdexcept>
#include "../common.h"
#include "../ndarray/ndarray_view.h"
#include "../ndarray/ndarray_shared_ring_base.h"
#include "media_node_io_base.h"

namespace mf {

class media_node_base;

/// Output of a media node.
template<std::size_t Dim, typename T>
class media_node_output : public media_node_output_base {
public:
	using frame_shape_type = typename frame_view_type::shape_type;

	using ring_type = ndarray_shared_ring<Dim, T>;

public:
	ndsize<Dim> frame_shape_;

	std::unique_ptr<ring_type> buffer_; ///< Ring buffer containing the output frames.
	frame_view_type view_;
	
public:
	explicit media_node_output(media_node_base& node) :
		media_node_output_base(node) { }
		
	void define_frame_shape(const frame_shape_type& shp);
	bool frame_shape_is_defined() const override { return frame_shape_.product() != 0; }
	const frame_shape_type& frame_shape() const;
	
	/// Set up the node output.
	/** Frame shape and required buffer duration, and stream duration (if seekable) must have been defined.
	 ** Creates the ring buffer. */
	void setup() override;
		
	ring_type& ring() { return *buffer_; }
	
	time_unit begin_write() override {
		auto view = ring().begin_write(1);
		view_ = view[0];
		return view.start_time();
	}
	
	void end_write(bool is_last_frame) override {
		ring().end_write(1);
	}

	#ifndef NDEBUG
	void debug_print(std::ostream&) const override;
	#endif
};

}

#include "media_node_output.tcc"

#endif
