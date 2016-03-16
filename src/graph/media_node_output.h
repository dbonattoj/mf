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
	
	void initialize();
		
	frame_view_type view() const { return view_; }
				
	void begin_write() override;
	void end_write(bool eof) override;

	void pull(time_unit target_time);
	full_view_type begin_read_span(time_span);
	void end_read(bool consume_frame);
	bool reached_end() const;
};

}

#include "media_node_output.tcc"

#endif
