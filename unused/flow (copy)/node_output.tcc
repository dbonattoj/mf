#include <sstream>
#include "node_output.h"

#include <iostream>

namespace mf { namespace flow {

template<std::size_t Dim, typename T>
void node_output<Dim, T>::define_frame_shape(const frame_shape_type& shp) {
	frame_shape_ = shp;
}


template<std::size_t Dim, typename T>
auto node_output<Dim, T>::frame_shape() const -> const frame_shape_type& {
	if(frame_shape_.product() == 0) throw std::logic_error("frame shape not defined");
	return frame_shape_;
}


template<std::size_t Dim, typename T>
void node_output<Dim, T>::setup() {
	assert(required_buffer_duration_is_defined());
	assert(frame_shape_is_defined());

	buffer_.reset(new ring_type(
		frame_shape_,
		required_buffer_duration(),
		node_.is_seekable(),
		node_.stream_duration())
	);
}


#ifndef NDEBUG
template<std::size_t Dim, typename T>
void node_output<Dim, T>::debug_print(std::ostream& str) const {
	str << "[frame_shape=" << frame_shape_
		<< ", required_duration=" << required_buffer_duration()
		<< ", setup=" << (buffer_ != nullptr)
		<< "]" << std::endl;

	if(buffer_ != nullptr) {
		str << "buffer: ";
		buffer_->debug_print(str);
		str << std::endl;
	}
}
#endif

}}
