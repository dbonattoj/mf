#ifndef MF_FLOW_FILTER_CONNECTOR_H_
#define MF_FLOW_FILTER_CONNECTOR_H_

#include "../flow/node.h"
#include "../ndarray/ndarray_view_cast.h"
#include "../ndarray/generic/ndarray_timed_view_generic.h"

class filter;

namespace mf { namespace flow {

/// Polymorphic base class for \ref filter_base.
template<std::size_t Input_dim, typename Input_elem>
class filter_connector_base : public node_remote_output {
public:
	using frame_shape_type = ndsize<Input_dim>;
		
	virtual const frame_shape_type& frame_shape() = 0;
};


/// Connector between filter input, and output of remote filter.
/** Enables connection between input and output of differing ndarray dimension and element type without conversion,
 ** through copy-less `ndarray_view_cast` operations.
 ** Derives from \ref node_remote_output, and performs the cast by intercepting the generic frames passing from the
 ** the node output to the connected node input. */
template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
class filter_connector : public filter_connector_base<Input_dim, Input_elem> {
	using base = filter_connector_base<Input_dim, Input_elem>;

private:
	using remote_output_type = filter::output_port<Output_dim, Output_elem>;
	using frame_shape_type = typename remote_output_type::frame_shape_type;
	
	remote_output_type& remote_output_;

public:
	filter_connector(remote_output_type& output) :
		remote_output_(output) { }

	node_output& this_output() noexcept override {
		return remote_output_.this_node_output();
	}

	const frame_shape_type& frame_shape() override {
		// TODO support for differing input/output dimension
		static_assert(Input_dim == Output_dim, "");
		return remote_output_.frame_shape();
	}
	
	void pull(time_span span) override {
		this_output().pull(span);
	}
	
	timed_frame_array_view begin_read(time_unit duration) override {
		auto generic_output_view = this_output().begin_read(duration);
		if(generic_output_view.is_null()) return timed_frame_array_view::null();
		
		auto generic_shape = make_ndsize(generic_output_view.duration());
		auto concrete_output_view = from_generic<Output_dim, Output_elem>(generic_output_view, generic_shape);
		
		using concrete_input_view_type = ndarray_timed_view<Input_dim, Input_elem>;
		auto concrete_input_view = ndarray_view_cast<concrete_input_view_type>(concrete_output_view);
		
		return to_generic<1>(concrete_input_view);
	}
	
	void end_read(time_unit duration) override {
		this_output().end_read(duration);
	}
	
	time_unit end_time() const override {
		return remote_output_.this_node_output().end_time();
	}
};

}}

#endif
