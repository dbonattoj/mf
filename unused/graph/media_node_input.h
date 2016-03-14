#ifndef MF_MEDIA_NODE_INPUT_H_
#define MF_MEDIA_NODE_INPUT_H_

#include "common.h"
#include "ndarray_view.h"

namespace mf {

class media_node;
template<std::size_t Dim, typename T> class media_node_output;

namespace detail {
	class media_node_input_base {
	public:	
		virtual void pull(time_unit) = 0;
		virtual void begin_read(time_unit) = 0;
		virtual void end_read(time_unit) = 0;
	};
}


/// Input of a media node.
template<std::size_t Dim, typename T>
class media_node_input : public detail::media_node_input_base {
public:
	using output_type = media_node_output<Dim, T>;
	
	using frame_view_type = ndarray_view<Dim, T>;
	using full_view_type = ndarray_view<Dim + 1, T>;
	
private:
	media_node& node_; ///< Media node that input belongs to.

	output_type* connected_output_ = nullptr; ///< Output of other media node to which input is connected.
	time_unit past_window_ = 0; ///< Number of frames preceding current frame that input will see.
	time_unit future_window_ = 0; ///< Number of frames succeeding current frame that input will see.
	
	full_view_type view_;
	std::ptrdiff_t view_center_;

public:
	media_node_input(media_node& node, time_unit past_window = 0, time_unit future_window = 0) :
		node_(node), past_window_(past_window), future_window_(future_window) { }
	
	void connect(output_type& output)  {
		connected_output_ = &output;
	}
		
	/// Get readable view of current frame.
	/** Only callable while in media_node::process_(). */
	frame_view_type view() const { return view_[view_center_]; }
	
	const full_view_type& full_view() const { return view_; }	
	std::ptrdiff_t full_view_center() const { return view_center_; }
	full_view_type past_view() const { return view_(0, view_center_ + 1, -1); }
	full_view_type future_view() const { return view_(view_center_, -1); }

	void pull() override;
	
	void begin_read() override;
	void end_read() override;
};	

}

#include "media_node_input.tcc"

#endif
