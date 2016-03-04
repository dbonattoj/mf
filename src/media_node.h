#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include <vector>
#include <memory>
#include <functional>
#include "ndcoord.h"
#include "ndarray_ring.h"

namespace mf {


class media_node_input_base {
public:
	virtual void pull() = 0;
};


class media_node_base {
protected:
	int time_;
	std::vector<std::unique_ptr<media_node_input_base>> inputs_;
	
	virtual void process_frame_() = 0;
};


template<std::size_t Dim, typename T>
class media_node_input : public media_node_input_base {
public:
	using connected_node_type = media_node<Dim, T>;

private:
	connected_node_type& connected_node_;

public:
	using input_view_type = typename connected_node_type::output_view_type;
	
	input_view_type input_view;
	
	explicit media_node_input(connected_node_type& nd) :
		connected_node_(nd) { }
		
	void pull() override {
		connected_node_.pull([&](const input_view_type& vw) {
			input_view.reset(vw);
		});
	}
};


template<std::size_t Dim, typename T>
class media_node : public media_node_base {
protected:
	ndarray_ring<Dim, T> output_buffer_;
	
public:
	using output_view_type = ndarray_view<Dim + 1, const T>;
	using read_function_ = void(const output_view_type& view, std::ptrdiff_t current_time_index);

	explicit media_node(const ndshape<Dim>& frame_shape) :
		output_buffer_(frame_shape, 1) { }

	template<std::size_t Dim2, typename T2>
	media_node_input<Dim2, T2>& add_input_(media_node<Dim2, T2>& connected_node) {
		auto ptr = std::make_unique<media_node_input<Dim2, T2>>(connected_node);
		inputs_.push_back(*ptr);
		return *ptr;
	}
	
	void initialize(std::size_t buffer_size);
	
	output_view_type pull(const std::function<read_function_type>& callback) {
		for(auto&& input : inputs_) input->pull();
		this->process_frame_();
		output_view_type out;
	}
};



}

#include "media_node.tcc"

#endif
