#include <sstream>
#include "media_node_output.h"
#include "../ndarray/ndarray_seekable_shared_ring.h"
#include "../ndarray/ndarray_forward_shared_ring.h"

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

	if(stream_duration_is_defined()) {
		MF_DEBUG("ndarray_seekable_shared_ring");
		buffer_.reset(new ndarray_seekable_shared_ring<Dim, T>(frame_shape_, required_buffer_duration(), stream_duration()));
	} else {
		MF_DEBUG("ndarray_forward_shared_ring");
		buffer_.reset(new ndarray_forward_shared_ring<Dim, T>(frame_shape_, required_buffer_duration()));
	}
}


#ifndef NDEBUG
template<std::size_t Dim, typename T>
void media_node_output<Dim, T>::debug_print(std::ostream& str) const {
	str << "[frame_shape=" << frame_shape_
		<< ", required_duration=" << required_buffer_duration()
		<< ", setup=" << (buffer_ != nullptr)
		<< "]" << std::endl;

	if(buffer_ != nullptr) {
		str << "buffer: capacity=" << buffer_->capacity()
			<< ", readable=" << buffer_->readable_time_span()
			<< ", writable=" << buffer_->writable_time_span()
			<< ", read_start_time=" << buffer_->read_start_time()
			<< ", write_start_time=" << buffer_->write_start_time() << std::endl;
	}
}
#endif

}
