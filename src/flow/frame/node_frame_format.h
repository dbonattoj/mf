#ifndef MF_FLOW_NODE_FRAME_FORMAT_H_
#define MF_FLOW_NODE_FRAME_FORMAT_H_

#include "../../nd/opaque_format/opaque_multi_ndarray_format.h"

namespace mf { namespace flow {

class node_frame_format : public opaque_multi_ndarray_format {
	using base = opaque_multi_ndarray_format;

public:
	using channel_index = std::ptrdiff_t;
	using parameter_index = std::ptrdiff_t;

	struct channel_data {
		const ndarray_format& format;
		ndarray_view_opaque<1> view;
	};
	
private:
	std::vector<std::ptrdiff_t> channel_parts_;
	std::vector<std::ptrdiff_t> parameter_parts_;

	std::ptrdiff_t selected_channel_part_index_ = -1;	

	shared_parameter_value_ptr* parameter_value_ptr_(parameter_index, frame_ptr) const;

public:
	node_frame_format();
	
	void select_channel(channel_index);
	channel_array_view_type channel_array_view(channel_index, frame_ptr) const;
	channel_const_array_view_type channel_array_view(channel_index, const_frame_ptr) const;

	shared_parameter_value_ptr parameter_value_ptr(parameter_index, frame_ptr) const;	
	void set_parameter_value_ptr(parameter_index, frame_ptr, shared_parameter_value_ptr) const;
	
	bool compare(const opaque_format&) const override;
	
	channel_index add_channel(const ndarray_format&);
	parameter_index add_parameter();
	
	bool has_array_format() const override;
	ndarray_format array_format() const override;
	std::ptrdiff_t array_offset() const override;
	
	void construct_frame(frame_ptr) const override;
	void destruct_frame(frame_ptr) const override;
	void initialize_frame(frame_ptr) const override;
};

}}

#endif
