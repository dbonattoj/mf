#include "../ndarray/ndarray_cast.h"

namespace mf { namespace flow {


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
time_unit filter_cast_connector<Input_dim, Input_elem, Output_dim, Output_elem>::end_time() const noexcept {
	return node_output_.end_time();
}
	

template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
node::pull_result filter_cast_connector<Input_dim, Input_elem, Output_dim, Output_elem>::pull(time_span& span, bool reconnect) {
	return node_output_.pull(span, reactivate);
}


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
timed_frame_array_view filter_cast_connector<Input_dim, Input_elem, Output_dim, Output_elem>::begin_read(time_unit duration) {
	auto generic_output_view = node_output_.begin_read(duration);
	if(generic_output_view.is_null()) return timed_frame_array_view::null();
			
	auto concrete_output_view = from_generic<Output_dim + 1, Output_elem>(generic_output_view, frame_shape());

	using concrete_input_view_type = ndarray_timed_view<Input_dim + 1, Input_elem>;
	auto concrete_input_view = ndarray_view_cast<concrete_input_view_type>(concrete_output_view);

	return to_generic<1>(concrete_input_view);
}


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
void filter_cast_connector<Input_dim, Input_elem, Output_dim, Output_elem>::end_read(time_unit duration) {
	node_output_.end_read(duration);
}


}}
