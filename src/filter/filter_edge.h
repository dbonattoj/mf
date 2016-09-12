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
#include "../flow/processing/processing_node.h"
#include "../flow/processing/sync_node.h"
#include "../nd/ndarray_timed_view.h"
#include <type_traits>
#include <utility>

namespace mf { namespace flow {

class filter;

template<std::size_t Output_dim, typename Output_elem> class filter_output;
template<std::size_t Input_dim, typename Input_elem> class filter_input;

template<std::size_t Input_dim, typename Input_elem>
class filter_edge_input_base {
public:
	using input_type = filter_input<Input_dim, Input_elem>;
	using input_frame_shape_type = ndsize<Input_dim>;
	using input_full_view_type = ndarray_timed_view<Input_dim + 1, Input_elem>;

	virtual const filter& origin_filter() const = 0;
	virtual void set_node_input(node_input&) = 0;
	virtual const input_frame_shape_type& input_frame_shape() const = 0;
		
	virtual input_full_view_type cast_connected_node_output_view(const timed_frame_array_view&) const = 0;
};



template<std::size_t Output_dim, typename Output_elem>
class filter_edge_output_base {
public:
	using output_type = filter_output<Output_dim, Output_elem>;
	using output_frame_shape_type = ndsize<Output_dim>;

	virtual const filter& destination_filter() const = 0;
	virtual void set_node_output(node_output&, std::ptrdiff_t channel_index) = 0;
	virtual std::ptrdiff_t node_output_channel_index() const = 0;
};


/// Edge in filter graph.
/** Connects output of *origin filter* (type Output_elem) to input of *destination filter* (type Input_elem). */
template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem>
class filter_edge :
	public filter_edge_input_base<Dim, Input_elem>,
	public filter_edge_output_base<Dim, Output_elem>
{	
	using base_in = filter_edge_input_base<Dim, Input_elem>;
	using base_out = filter_edge_output_base<Dim, Output_elem>;
	
public:
	using typename base_in::input_type;
	using typename base_in::input_frame_shape_type;
	using typename base_in::input_full_view_type;
	using typename base_out::output_type;
	using typename base_out::output_frame_shape_type;

	using casted_full_view_type = ndarray_timed_view<Dim + 1, Casted_elem>;

private:
	input_type& input_;
	output_type& output_;
	
	node_input* node_input_ = nullptr; // TODO remove reference to nodes (allow filter edge == multiple node edges)
	node_output* node_output_ = nullptr;
	std::ptrdiff_t node_output_channel_index_ = -1;
	
	bool is_connected_() const noexcept { return (node_input_ != nullptr) && (node_output_ != nullptr); }
	
protected:		
	filter_edge(input_type& in, output_type& out) :
		input_(in), output_(out) { }
	filter_edge(filter_edge&&) = default;
	filter_edge& operator=(filter_edge&&) = default;
	
	casted_full_view_type output_view_to_casted_view_(const timed_frame_array_view& generic_output_view) const;
	
	virtual void install_(graph&) = 0;

public:	
	virtual ~filter_edge() = default;

	void set_node_input(node_input& in) override;
	void set_node_output(node_output& out, std::ptrdiff_t channel_index) override;
	
	const filter& origin_filter() const override { return output_.this_filter(); }
	const filter& destination_filter() const override { return input_.this_filter(); }
	
	node_graph& this_node_graph();
	node_input& this_node_input() { Assert(node_input_ != nullptr); return *node_input_; }
	const node_input& this_node_input() const { Assert(node_input_ != nullptr); return *node_input_; }
	node_output& this_node_output() { Assert(node_output_ != nullptr); return *node_output_; }
	const node_output& this_node_output() const { Assert(node_output_ != nullptr); return *node_output_; }
	std::ptrdiff_t node_output_channel_index() const override
		{ Assert(node_output_ != nullptr); return node_output_channel_index_; }

	const input_frame_shape_type& input_frame_shape() const override { return output_.frame_shape(); }
	const output_frame_shape_type& output_frame_shape() const { return output_.frame_shape(); }
};


/// Edge in filter graph without conversion.
template<std::size_t Dim, typename Output_elem, typename Input_elem>
class filter_direct_edge final :
	public filter_edge<Dim, Output_elem, Input_elem, Input_elem>
{
	using base = filter_edge<Dim, Output_elem, Input_elem, Input_elem>;

	/*
	origin node
	 [output_]   (Output_elem)
		|
		|  ndarray_view_cast: Output_elem -> Casted_elem
		v
	 [input_]    (Input_elem = Casted_elem)
	destination node
	*/

public:
	using typename base::input_type;
	using typename base::input_frame_shape_type;
	using typename base::input_full_view_type;
	using typename base::output_type;
	using typename base::output_frame_shape_type;

protected:
	void install_(node_graph&) override;

public:
	filter_direct_edge(input_type& in, output_type& out) :
		base(in, out) { }

	input_full_view_type cast_connected_node_output_view(const timed_frame_array_view&) const override;
};


// TODO allow one filter edge for multiple node edges

/// Edge in filter graph with conversion.
template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem, typename Convert_function>
class filter_converting_edge final :
	public filter_edge<Dim, Output_elem, Casted_elem, Input_elem>,
	public processing_node_handler
{
	using base = filter_edge<Dim, Output_elem, Casted_elem, Input_elem>;

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
	using typename base::input_full_view_type;
	using typename base::output_type;
	using typename base::output_frame_shape_type;
	
private:
	Convert_function convert_function_;
	sync_node* convert_node_ = nullptr;
	sync_node::output_channel_type* convert_node_output_channel_ = nullptr; 

protected:
	void install_(node_graph&) override;

public:
	filter_converting_edge(input_type& in, output_type& out, Convert_function&& func) :
		base(in, out), convert_function_(std::forward<Convert_function>(func)) { }

	void handler_setup(processing_node&) final override;
	void handler_pre_process(processing_node&, processing_node_job&) final override;
	void handler_process(processing_node&, processing_node_job&) final override;

	input_full_view_type cast_connected_node_output_view(const timed_frame_array_view&) const override;
};


}}

#include "filter_edge.tcc"

#endif
