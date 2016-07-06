/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "../ndarray/ndarray_view_cast.h"

namespace mf { namespace flow {


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
void filter_edge<Input_dim, Input_elem, Output_dim, Output_elem>::install_() {
	Expects(node_input_ != nullptr && node_output_ != nullptr);
	cast_connector_.reset(new cast_connector(*this));
	node_input_->connect(*cast_connector_);
}


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
void filter_edge<Input_dim, Input_elem, Output_dim, Output_elem>::set_node_input(node_input& in) {
	Expects(node_input_ == nullptr);
	node_input_ = &in;
	if(node_input_ != nullptr && node_output_ != nullptr) install_();
}


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
void filter_edge<Input_dim, Input_elem, Output_dim, Output_elem>::set_node_output(node_output& out) {
	Expects(node_output_ == nullptr);
	node_output_ = &out;
	if(node_input_ != nullptr && node_output_ != nullptr) install_();
}


///////////////

template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
class filter_edge<Input_dim, Input_elem, Output_dim, Output_elem>::cast_connector : public node_remote_output {
private:
	filter_edge& edge_;

public:
	explicit cast_connector(filter_edge& edge) : edge_(edge) { }
	
	node_output& this_output() noexcept override {
		return edge_.this_node_output();
	}
	
	time_unit end_time() const noexcept override {
		return edge_.this_node_output().end_time();	
	}
	
	node::pull_result pull(time_span& span, bool reconnect) override {
		return edge_.this_node_output().pull(span, reconnect);
	}
	
	timed_frame_array_view begin_read(time_unit duration) override {
		auto generic_output_view = edge_.this_node_output().begin_read(duration);
		if(generic_output_view.is_null()) return timed_frame_array_view::null();
				
		auto concrete_output_view = from_generic<Output_dim + 1, Output_elem>(generic_output_view, edge_.output_frame_shape());
	
		using concrete_input_view_type = ndarray_timed_view<Input_dim + 1, Input_elem>;
		auto concrete_input_view = ndarray_view_cast<concrete_input_view_type>(concrete_output_view);
	
		return to_generic<1>(concrete_input_view);
	}

	void end_read(time_unit duration) override {
		edge_.this_node_output().end_read(duration);
	}
};

}}
