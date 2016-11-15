#include "node_frame_format.h"

namespace mf { namespace flow {

namespace {
	static constexpr std::size_t parameter_ptr_size = sizeof(shared_parameter_value_ptr);
	static constexpr std::size_t parameter_ptr_alignment_requirement = alignof(shared_parameter_value_ptr);
}


node_frame_format::node_frame_format() { }


void node_frame_format::select_channel(channel_index channel_idx) {
	selected_channel_part_index_ = channel_parts_.at(channel_idx);
}


channel_array_view_type node_frame_format::channel_array_view(channel_index channel_idx, frame_ptr frm) const {
	std::ptrdiff_t part_idx = channel_parts_.at(channel_idx);
	return base::part_array_view(part_idx, frm);
}


channel_const_array_view_type node_frame_format::channel_array_view(channel_index channel_idx, const_frame_ptr frm) const {
	std::ptrdiff_t part_idx = channel_parts_.at(channel_idx);
	return base::part_array_view(part_idx, frm);
}


shared_parameter_value_ptr* node_frame_format::parameter_value_ptr_(parameter_index parameter_idx, frame_ptr frm) const {
	std::ptrdiff_t part_idx = channel_parts_.at(channel_idx);
	std::ptrdiff_t offset = base::part_at(part_idx).offset;
	return reinterpret_cast<const shared_parameter_value_ptr*>(advance_raw_ptr(frm, offset));
}


shared_parameter_value_ptr node_frame_format::parameter_value_ptr(parameter_index parameter_idx, frame_ptr frm) const {
	return *parameter_value_ptr_(parameter_idx, frm);
}


void node_frame_format::set_parameter_value_ptr
(parameter_index parameter_idx, frame_ptr frm, shared_parameter_value_ptr new_ptr) const {
	*parameter_value_ptr_(parameter_idx, frm) = new_ptr;
}



bool node_frame_format::compare(const opaque_format& frm) const {
	if(typeid(node_frame_format) == typeid(frm)) return (static_cast<const node_frame_format&>(frm) == *this);
	else return false;
}


auto node_frame_format::add_channel_part(const ndarray_format& arr) -> channel_index {
	const part& ptr = base::add_part(arr);
	std::ptrdiff_t part_idx = parts_count() - 1;
	channel_parts_.push_back(part_idx);
	return channel_parts_.size() - 1;
}


auto node_frame_format::add_parameter_part() -> parameter_index {
	ndarray_format arr = make_raw_ndarray_format(parameter_ptr_size, parameter_ptr_alignment_requirement);
	const part& ptr = base::add_part(arr);
	std::ptrdiff_t part_idx = parts_count() - 1;
	parameter_parts_.push_back(part_idx);
	return parameter_parts_.size() - 1;
}


bool node_frame_format::has_array_format() const {
	return (selected_channel_part_index_ >= 0);
}


ndarray_format node_frame_format::array_format() const {
	Assert(has_array_format());
	return part_at(selected_channel_part_index_).format;
}


std::ptrdiff_t node_frame_format::array_offset() const {
	Assert(has_array_format());
	return part_at(selected_channel_part_index_).offset;
}


void node_frame_format::construct_frame(frame_ptr frm) const {
	for(parameter_index parameter_idx : parameter_parts_) {
		shared_parameter_value_ptr* ptr = parameter_value_ptr_(parameter_idx, frm);
		new (static_cast<void*>(ptr)) shared_parameter_value_ptr(nullptr);
	}
}


void node_frame_format::destruct_frame(frame_ptr) const {
	for(parameter_index parameter_idx : parameter_parts_)
		parameter_value_ptr_(parameter_idx, frm)->~shared_parameter_value_ptr();
}


void node_frame_format::initialize_frame(frame_ptr) const {
	for(parameter_index parameter_idx : parameter_parts_)
		parameter_value_ptr_(parameter_idx, frm)->reset();
}



}}
