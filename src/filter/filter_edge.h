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

class filter_output_base;
class filter_input_base;
template<std::size_t Output_dim, typename Output_elem> class filter_output;
template<std::size_t Input_dim, typename Input_elem> class filter_input;

/// Edge in filter graph, base class for input.
/** Templated for concrete frame type of destination filter's input port. */
template<std::size_t Input_dim, typename Input_elem>
class filter_edge_input_base {
public:
	using input_type = filter_input<Input_dim, Input_elem>;
	using input_frame_shape_type = ndsize<Input_dim>;
	using input_full_view_type = ndarray_timed_view<Input_dim + 1, Input_elem>;

	virtual filter_output_base& origin() const = 0;
	virtual filter& origin_filter() const = 0;
	virtual const input_frame_shape_type& input_frame_shape() const = 0;
		
	virtual input_full_view_type cast_connected_node_output_view(const timed_frame_array_view&) const = 0;
	
	virtual void install(node_output& origin_node_output, std::ptrdiff_t origin_node_channel_index, node_input& destination_node_input) = 0;
};


/// Edge in filter graph, base class for output.
/** Templated for concrete frame type of source filter's output port. */
template<std::size_t Output_dim, typename Output_elem>
class filter_edge_output_base {
public:
	using output_type = filter_output<Output_dim, Output_elem>;
	using output_frame_shape_type = ndsize<Output_dim>;
	
	virtual filter_input_base& destination() const = 0;
	virtual filter& destination_filter() const = 0;
};


/// Edge in filter graph.
/** Connects output of *origin filter* (type `Output_elem`) to input of *destination filter* (type `Input_elem`).
 ** A connected \ref filter_input owns one \ref filter_edge. A connected \ref filter_output references one of multiple
 ** \ref filter_edge.
 ** Instantiated as one of its two non-abstract subclasses \ref filter_direct_edge or \ref filter_converting_edge,
 ** where the latter will insert an additional processing node in the node graph for element-wise conversion.
 ** The ndarray of element type `Output_elem` from the output is first *casted* to an ndarray of element type
 ** `Casted_elem`, without copying data. For example `elem_tuple<rgb_color, float>` to `rgb_color`.
 ** For \ref filter_direct_edge, this casted ndarray is passed to the destination filter input, and
 ** `Input_elem == Casted_elem`.
 ** For \ref filter_converting_edge it is converted, to an ndarray of element type `Input_elem`, and this new ndarray
 ** is passed to the destination filter. */
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
	std::ptrdiff_t node_output_channel_ = -1;
		
protected:		
	filter_edge(input_type& in, output_type& out) :
		input_(in), output_(out) { }
	filter_edge(filter_edge&&) = default;
	filter_edge& operator=(filter_edge&&) = default;
	
	casted_full_view_type output_view_to_casted_view_(const timed_frame_array_view& generic_output_view) const;
	
public:	
	virtual ~filter_edge() = default;
	
	output_type& origin() const override { return output_; }
	filter& origin_filter() const override { return output_.this_filter(); }
	input_type& destination() const override { return input_; }	
	filter& destination_filter() const override { return input_.this_filter(); }
	
	std::ptrdiff_t node_output_channel() const { return node_output_channel_; }
	void set_node_output_channel(std::ptrdiff_t ch) { node_output_channel_ = ch; } 
	
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

public:
	filter_direct_edge(input_type& in, output_type& out) :
		base(in, out) { }

	input_full_view_type cast_connected_node_output_view(const timed_frame_array_view&) const override;
	
	void install(node_output& origin_node_output, std::ptrdiff_t origin_node_channel_index, node_input& destination_node_input) override;
};


/// Edge in filter graph with conversion.
/** Inserts intermediary \ref sync_node into the node graph which does element-wise conversion of the ndarray from
 ** `Casted_elem` to `Input_elem`. The function object of type `Convert_function` is called on each element. */
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

public:
	filter_converting_edge(input_type& in, output_type& out, Convert_function&& func) :
		base(in, out), convert_function_(std::forward<Convert_function>(func)) { }

	void handler_pre_process(processing_node&, processing_node_job&) final override;
	void handler_process(processing_node&, processing_node_job&) final override;

	input_full_view_type cast_connected_node_output_view(const timed_frame_array_view&) const override;
	
	void install(node_output& origin_node_output, std::ptrdiff_t origin_node_channel_index, node_input& destination_node_input) override;
};


}}

#include "filter_edge.tcc"

#endif
