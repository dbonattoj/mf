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

#ifndef MF_FLOW_FILTER_EDGE_H_
#define MF_FLOW_FILTER_EDGE_H_

#include "../utility/misc.h"
#include "../flow/processing_node.h"
#include "../flow/sync_node.h"
#include <type_traits>
#include <utility>

namespace mf { namespace flow {

template<std::size_t Output_dim, typename Output_elem> class filter_output;
template<std::size_t Input_dim, typename Input_elem> class filter_input;

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
public:
	using output_type = filter_output<Output_dim, Output_elem>;
	using output_frame_shape_type = ndsize<Output_dim>;

	virtual void set_node_output(node_output&) = 0;
	virtual const output_frame_shape_type& output_frame_shape() const = 0;
};



template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem = Casted_elem>
class filter_edge :
	public filter_edge_input_base<Dim, Input_elem>,
	public filter_edge_output_base<Dim, Output_elem>
{	
	using base_in = filter_edge_input_base<Dim, Input_elem>;
	using base_out = filter_edge_output_base<Dim, Output_elem>;

	/*
	origin node
	 [output_]   (Output_type)
		|
		|  ndarray_view_cast: Output_type -> Casted_type
		v
	 [input_]    (Input_type = Casted_type)
	destination node
	*/
	
public:
	using typename base_in::input_type;
	using typename base_in::input_frame_shape_type;
	using typename base_out::output_type;
	using typename base_out::output_frame_shape_type;

private:
	class cast_connector;

	input_type& input_;
	output_type& output_;
	
	node_input* node_input_ = nullptr;
	node_output* node_output_ = nullptr;
	std::unique_ptr<cast_connector> cast_connector_;
	
protected:
	virtual void install_(graph&);
	
	void create_cast_connector_();
	cast_connector& this_cast_connector_() { Expects(cast_connector_); return *cast_connector_; }
	const cast_connector& this_cast_connector_() const { Expects(cast_connector_); return *cast_connector_; }
	
public:
	filter_edge(input_type& in, output_type& out) :
		input_(in), output_(out) { }
	filter_edge(filter_edge&&) = default;
	filter_edge& operator=(filter_edge&&) = default;
	virtual ~filter_edge() = default;
		
	void set_node_input(node_input& in) override;
	void set_node_output(node_output& out) override;
	
	graph& this_graph();
	node_input& this_node_input() { Expects(node_input_ != nullptr); return *node_input_; }
	const node_input& this_node_input() const { Expects(node_input_ != nullptr); return *node_input_; }
	node_output& this_node_output() { Expects(node_output_ != nullptr); return *node_output_; }
	const node_output& this_node_output() const { Expects(node_output_ != nullptr); return *node_output_; }

	const input_frame_shape_type& input_frame_shape() const override { return output_.frame_shape(); }
	const output_frame_shape_type& output_frame_shape() const override { return output_.frame_shape(); }
};


template<std::size_t Dim, typename Input_elem, typename Casted_elem, typename Output_elem, typename Convert_function>
class filter_converting_edge :
	public filter_edge<Dim, Input_elem, Casted_elem, Output_elem>,
	public processing_node_handler
{
	using base = filter_edge<Dim, Input_elem, Casted_elem, Output_elem>;

	/*
	origin node
	 [output_]   (Output_type)
		|
		|  ndarray_view_cast: Output_type -> Casted_type
		v
	 [in]        (Casted_type)
	converter node: Input_type = Convert_function(Casted_type)
	 [out]       (Input_type)
		|
		|  direct
		v
	 [input_]    (Input_type)
	destination node
	*/

public:
	using typename base::input_type;
	using typename base::input_frame_shape_type;
	using typename base::output_type;
	using typename base::output_frame_shape_type;
	
private:
	Convert_function convert_function_;
	sync_node* convert_node_ = nullptr;

protected:
	void install_(graph&) override;

public:
	filter_converting_edge(input_type& in, output_type& out, Convert_function&& func) :
		base(in, out), convert_function_(std::forward<Convert_function>(func)) { }

	void handler_setup() final override;
	void handler_pre_process(processing_node_job&) final override;
	void handler_process(processing_node_job&) final override;
};



}}

#include "filter_edge.tcc"

#endif
