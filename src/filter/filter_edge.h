#ifndef MF_FLOW_FILTER_EDGE_H_
#define MF_FLOW_FILTER_EDGE_H_

#include "filter_cast_connector.h"
#include <type_traits>

namespace mf { namespace flow {
	

template<std::size_t Input_dim, typename Input_elem>
class filter_edge_input_base {
public:
	using input_type = filter_input<Input_dim, Input_elem>;
	using input_frame_shape_type = ndsize<Input_dim>;

	virtual void set_node_input(node_input&) = 0;
	virtual const input_frame_shape_type& input_frame_shape() const = 0;
};



template<std::size_t Output_dim, typename Output_elem>
class filter_edge_output_base {
	using output_type = filter_output<Output_dim, Output_elem>;
	using output_frame_shape_type = ndsize<Output_dim>;

	virtual void set_node_output(node_output&) = 0,
	virtual const output_frame_shape_type& output_frame_shape() const = 0;
};



template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
class filter_edge :
	public filter_edge_input_base<Input_dim, Input_elem>,
	public filter_edge_output_base<Output_dim, Output_elem>
{
	static_assert(Input_dim == Output_dim, "edge input and output dimension must be same (for now)");
	
	using base_in = filter_edge_input_base<Input_dim, Input_elem>;
	using base_out = filter_edge_output_base<Output_dim, Output_elem>;
	
public:
	using typename base_in::input_type;
	using typename base_in::input_frame_shape_type;
	using typename base_out::output_type;
	using typename base_out::output_frame_shape_type;
	using cast_connector_type = filter_cast_connector<Input_dim, Input_elem, Output_dim, Output_elem>;

	
private:
	input_type& input_;
	output_type& output_;
	
	node_input* node_input_ = nullptr;
	node_output* node_output_ = nullptr;
	std::unique_ptr<cast_connector_type> cast_connector_;
	
	void install_();
	
public:
	filter_edge(input_type& in, output_type& out) :
		input_(in), output_(out) { }
		
	void set_node_input(node_input& in) override { node_input_ = &in; }
	void set_node_output(node_output& out) override { node_output_ = &out; }

	const input_frame_shape_type& input_frame_shape() const override { return output_.frame_shape(); }
	const output_frame_shape_type& output_frame_shape() const override { return output_.frame_shape(); }
};




}}

#include "filter_edge.tcc"

#endif
