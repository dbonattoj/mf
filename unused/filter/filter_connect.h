#ifndef MF_FLOW_FILTER_CONNECT_H_
#define MF_FLOW_FILTER_CONNECT_H_

#include "filter.h"

namespace mf { namespace flow {

template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
struct filter_implicit_converter {
	using type = void;
}


template<std::size_t Input_dim, typename Input_elem>
void connect(filter::input_port<Input_dim, Input_elem>& in, filter::output_port<Input_dim, Input_elem>& out);


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
void connect(filter::input_port<Input_dim, Input_elem>& in, filter::output_port<Output_dim, Output_elem>& out);


}}

#include "filter_connect.tcc"

#endif
