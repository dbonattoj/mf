#ifndef MF_MEDIA_NODE_OUTPUT_H_
#define MF_MEDIA_NODE_OUTPUT_H_

#include <memory>
#include <stdexcept>
#include "../common.h"
#include "../ndarray_view.h"
#include "../ndarray_shared_ring.h"
#include "media_node_io_base.h"

namespace mf {

class media_node;

/// Output of a media node.
template<std::size_t Dim, typename T>
class media_node_output : public media_node_output_base {
public:
	using frame_view_type = ndarray_view<Dim, T>;
	using full_view_type = ndarray_view<Dim + 1, T>;
	using frame_shape_type = typename frame_view_type::shape_type;

private:
	using buffer_type = ndarray_shared_ring<Dim, T>;

	ndsize<Dim> frame_shape_;

	std::unique_ptr<buffer_type> buffer_; ///< Ring buffer containing the output frames.
	frame_view_type view_;

public:
	explicit media_node_output(media_node& node) :
		media_node_output_base(node) { }
		
	void define_frame_shape(const frame_shape_type& shp);
	bool frame_shape_is_defined() const override { return frame_shape_.product() != 0; }
	const frame_shape_type& frame_shape() const;
	
	/// Set up the node input.
	/** Frame shape and required buffer duration must have been defined prior to this. Creates the ring buffer. */
	void setup() override;
	
	/// Return writable view to the new frame.
	/** Is accessed by node, between begin_write() and end_write() calls. */
	frame_view_type view() const { return view_; }
	
	/// Begin writing one frame into the output.
	void begin_write() override;
	
	/// End writing one frame into the output.
	/** If \a is_last_frame, this was the last frame. Then the end time is marked in the buffer. */
	void end_write(bool is_last_frame) override;



	// reading interface used by media_node_input only:

	/// Pull frames until \a target_time.
	/** Calls media_node::pull(). */
	void pull(time_unit target_time);

	/// Begin reading time span \a span from buffer.
	/** Returns the view to the frames for this time span. Returned view duration may be smaller than requested when
	 ** it reaches the end. */
	full_view_type begin_read_span(time_span span);
	
	/// End reading the frames from time span.
	/** If \a consume_frame, then for the next call to begin_read_span() the span must not include the first frame
	 ** it has for the previous call. Then the frame becomes writable in the buffer. */
	void end_read(bool consume_frame);
	
	/// Returns true when no more frame can be read.
	/** Must not call begin_read_span() after this returned true. */
	bool reached_end() const;
};

}

#include "media_node_output.tcc"

#endif
