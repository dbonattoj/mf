#ifndef MF_MEDIA_NODE_OUTPUT_H_
#define MF_MEDIA_NODE_OUTPUT_H_

#include <memory>
#include "common.h"
#include "ndarray_view.h"
#include "ndarray_shared_ring.h"

namespace mf {

class media_node;

namespace detail {
	class media_node_output_base {	
	public:
		virtual void begin_write() = 0;
		virtual void end_write() = 0;
	};
}


/// Output of a media node.
template<std::size_t Dim, typename T>
class media_node_output : public detail::media_node_output_base {
public:
	using frame_view_type = ndarray_view<Dim, T>;
	using full_view_type = ndarray_view<Dim + 1, T>;

private:
	using buffer_type = ndarray_shared_ring<Dim, T>;

	media_node& node_; ///< Media node that input belongs to.

	std::unique_ptr<buffer_type> buffer_; ///< Ring buffer containing the output frames.
	full_view_type view_;

public:
	explicit media_node_output(media_node& node) : node_(node) { }
	
	void initialize(const ndsize<Dim>& frame_shape, time_unit duration);
		
	frame_view_type view() const { return view_[0]; }
	
	void pull(time_unit target_time);
		
	read_view_type begin_read_span(time_span);
	void end_read(time_unit consumed_duration);
	
	void begin_write() override;
	void end_write(bool eof) override;
};

}

#include "media_node_output.tcc"

#endif
