#include "node_frame_view.h"

namespace mf { namespace flow {

node_frame_window_view extract_channel(const node_frame_window_view& vw, std::ptrdiff_t channel_index) {
	node_selected_channel_frame_format new_format(vw.frame_format(), channel_index);
	frame_array_view new_vw(vw.non_timed().base_view(), new_format);
	timed_frame_array_view new_vw2(new_vw, vw.start_time());
	return new_vw2;
	// TODO better view constructors
}

}}
