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
#include "../flow/graph.h"
#include <algorithm>

namespace mf { namespace flow {

template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem>
void filter_edge<Dim, Input_elem, Casted_elem, Output_elem>::create_cast_connector_() {
	Expects(node_input_ != nullptr && node_output_ != nullptr);	
	cast_connector_.reset(new cast_connector(*this));	
}


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem>
graph& filter_edge<Dim, Input_elem, Casted_elem, Output_elem>::this_graph() {
	Expects(node_input_ != nullptr);
	return node_input_->this_node().this_graph();
}


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem>
void filter_edge<Dim, Input_elem, Casted_elem, Output_elem>::install_(graph&) {
	create_cast_connector_();
	this_node_input().connect( this_cast_connector_() );
}


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem>
void filter_edge<Dim, Input_elem, Casted_elem, Output_elem>::set_node_input(node_input& in) {
	Expects(node_input_ == nullptr);
	node_input_ = &in;
	if(node_input_ != nullptr && node_output_ != nullptr) this->install_(this_graph());
}


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem>
void filter_edge<Dim, Input_elem, Casted_elem, Output_elem>::set_node_output(node_output& out) {
	Expects(node_output_ == nullptr);
	node_output_ = &out;
	if(node_input_ != nullptr && node_output_ != nullptr) this->install_(this_graph());
}


///////////////


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem>
class filter_edge<Dim, Input_elem, Casted_elem, Output_elem>::cast_connector : public node_remote_output {
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
				
		auto concrete_output_view = from_generic<Dim + 1, Output_elem>(generic_output_view, edge_.output_frame_shape());
	
		using concrete_casted_view_type = ndarray_timed_view<Dim + 1, Casted_elem>;
		auto concrete_casted_view = ndarray_view_cast<concrete_casted_view_type>(concrete_output_view);
	
		return to_generic<1>(concrete_casted_view);
	}

	void end_read(time_unit duration) override {
		edge_.this_node_output().end_read(duration);
	}
};


///////////////


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem, typename Convert_function>
void filter_converting_edge<Dim, Input_elem, Casted_elem, Output_elem, Convert_function>::install_(graph& gr) {
	base::create_cast_connector_();
	
	convert_node_ = &gr.add_node<sync_node>();
	convert_node_->set_handler(*this);
	node_input& convert_node_input = convert_node_->add_input();
	node_output& convert_node_output = convert_node_->add_output();
	
	convert_node_output.define_format( frame_format::default_format<Input_elem>() );
	
	convert_node_input.connect( base::this_cast_connector_() );
	base::this_node_input().connect( convert_node_output );
}


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem, typename Convert_function>
void filter_converting_edge<Dim, Input_elem, Casted_elem, Output_elem, Convert_function>::handler_setup() {
	node_input& convert_node_input = *convert_node_->inputs().front();
	node_output& convert_node_output = convert_node_->output();
	convert_node_output.define_frame_length( convert_node_input.connected_output().this_output().frame_length() );	
}


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem, typename Convert_function>
void filter_converting_edge<Dim, Input_elem, Casted_elem, Output_elem, Convert_function>::
handler_pre_process(processing_node_job& job) { }


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem, typename Convert_function>
void filter_converting_edge<Dim, Input_elem, Casted_elem, Output_elem, Convert_function>::
handler_process(processing_node_job& job) {
	auto in = job.input_view(0)[0];
	const auto& out = job.output_view();
	
	auto concrete_in = from_generic<Dim, Casted_elem>(in, base::input_frame_shape());
	auto concrete_out = from_generic<Dim, Input_elem>(out, base::input_frame_shape());
	
	std::transform(concrete_in.begin(), concrete_in.end(), concrete_out.begin(), convert_function_);
}


}}
