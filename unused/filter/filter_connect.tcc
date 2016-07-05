#include "filter_connect.h"
#include "../flow/graph.h"
#include "../flow/multiplex_node.h"
#include <type_traits>

namespace mf { namespace flow {

template<std::size_t Input_dim, typename Input_elem>
void connect(filter::input_port<Input_dim, Input_elem>& in, filter::output_port<Input_dim, Input_elem>& out) {
	Assert(&origin_node.this_graph == &destination_node.this_graph());
	Assert(! destination_node.is_connected(), "`in` must not be already connected");

	// origin_node
	// [out]
	//   v
	//   v  <-- establishing connection
	//   v
	// [in]
	// destination_node

	node& origin_node = out.this_node();
	node& destination_node = in.this_node();
	graph& gr = origin_node.this_graph();
	
	if(! out.this_node_output().is_connected()) {
		// `out` is not yet connected: directly connect `in` to `out`
		in.set_connector<filter_connector, Input_dim, Input_elem, Input_dim, Input_elem>(out);
	} else {
		// `out` is already connected: check if connected to multiplexer
		node_input& origin_connected_input = out.this_node_output().connected_input();
		if(dynamic_cast<multiplex_node*>(&origin_connected_input.this_node()) != nullptr) {
			// `out` is connected to a multiplexer: add output to multiplexer, and connect to it
			multiplex_node& mlpx_node = static_cast<multiplex_node&>(origin_connected_input.this_node());
			multiplex_node_output& mlpx_output = mlpx_node.add_output();
			in.set_connector<filter_multiplex_connector, Input_dim, Input_elem>(mlpx_output, out);

		} else {
			// `out` is connected to non-multiplexer
			origin_connected_input.disconnect();
			multiplex_node& mlpx_node = gr.add_node<multiplex_node>();
			multiplex_node_output& mlpx_output1 = mlpx_node.add_output();
			multiplex_node_output& mlpx_output2 = mlpx_node.add_output();
			mlpx_node.input().connect(origin_node);
			origin_connected_input.
		}
	}
}

/*
template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
void connect(filter::input_port<Input_dim, Input_elem>& in, filter::output_port<Output_dim, Output_elem>& out) {
	Assert(&origin_node.this_graph == &destination_node.this_graph());

	constexpr bool same_format = std::is_same<Input_elem, Output_elem>::value && (Input_dim == Output_dim);

	node& origin_node = out.this_node();
	node& destination_node = in.this_node();
	graph& gr = origin_node.this_graph();
	
	if(! in.is_connected()) {
		// `in` is not yet connected, connect directly
				
	}
}
*/

}}

multiplex_node_output& mplx_out, multiplex_input_type& mplx_in
