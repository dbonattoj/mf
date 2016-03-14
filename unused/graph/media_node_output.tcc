#include "media_node_input.h"

namespace mf {

template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::initialize(const ndsize<Dim>& frame_shape, time_unit duration) {
	buffer_.reset(new buffer_type(frame_shape, duration));
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::pull(time_unit target_time) {
	// pull frames from node until arriving at target time
	while(node_.current_time() < last_frame) node_.pull_frame();
}


template<std::size_t Dim, typename T>
auto media_node_output<Dim, T>::begin_read(time_span span) -> read_view_type {
	return buffer_.begin_read_span(span);
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::end_read(time_unit duration) {
	buffer_.end_read(duration);
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::begin_write() {
	auto view = buffer_.begin_write(1);
	view_.reset(view[0]);
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::end_write(bool eof) {
	buffer_.end_write(1, eof);
}

}