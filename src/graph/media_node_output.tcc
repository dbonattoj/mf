#include "media_node_output.h"

namespace mf {

template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::define_frame_shape(const frame_shape_type& shp) {
	frame_shape_ = shp;
}


template<std::size_t Dim, typename T>
auto media_node_output<Dim, T>::frame_shape() const -> const frame_shape_type& {
	if(frame_shape_.product() == 0) throw std::logic_error("frame shape not defined");
	return frame_shape_;
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::setup() {
	assert(required_buffer_duration_is_defined());
	assert(frame_shape_is_defined());

	buffer_.reset(new buffer_type(frame_shape_, buffer_duration_));
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::pull(time_unit target_time) {
	node_.pull(target_time);
}


template<std::size_t Dim, typename T>
auto media_node_output<Dim, T>::begin_read_span(time_span span) -> full_view_type {
	return buffer_->begin_read_span(span);
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::end_read(bool consume_frame) {
	buffer_->end_read(consume_frame ? 1 : 0);
}


template<std::size_t Dim, typename T>
bool media_node_output<Dim, T>::reached_end() const {
	return buffer_->reader_eof();
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::begin_write() {
	auto view = buffer_->begin_write(1);
	view_.reset(view[0]);
}


template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::end_write(bool is_last_frame) {
	buffer_->end_write(1, is_last_frame);
}

}