#ifndef MF_FLOW_NODE_IO_WRAPPER_H_
#define MF_FLOW_NODE_IO_WRAPPER_H_

#include "node.h"
#include "../queue/frame.h"

namespace mf { namespace flow {

/// Output port of flow graph node, with nd-array frame.
template<typename Output, std::size_t Dim, typename Elem>
class node_output_wrapper : public Output {
public:
	using node_type = typename Output::node_type;
	using elem_type = Elem;
	using frame_shape_type = ndsize<Dim>;
	constexpr static std::size_t dimension = Dim;

private:
	frame_shape_type frame_shape_;

public:
	explicit node_output_wrapper(node_type& nd) :
		Output(nd, frame_format::default_format<Elem>()) { }
	
	void define_frame_shape(const frame_shape_type& shp) {
		Output::define_frame_length(shp.product());
	}
	
	const frame_shape_type& frame_shape() { return frame_shape_; }
};


/// Input port of flow graph node, with nd-array frame.
template<typename Input, std::size_t Dim, typename Elem>
class node_input_wrapper : public Input {
public:
	using elem_type = Elem;
	using frame_shape_type = ndsize<Dim>;
	constexpr static std::size_t dimension = Dim;

private:
	frame_shape_type frame_shape_;

public:
	node_input_wrapper(node& nd, time_unit past_window = 0, time_unit future_window = 0) :
		Input(nd, past_window, future_window) { }
	
	const frame_shape_type& frame_shape() { return frame_shape_; }
	
	template<typename Output>
	void connect(node_output_wrapper<Output, Dim, Elem>& output) {
		Input::connect(output);
		frame_shape_ = output.frame_shape();
	}
};



}}

#endif
