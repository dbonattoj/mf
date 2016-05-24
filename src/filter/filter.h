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

#ifndef MF_FLOW_FILTER_H_
#define MF_FLOW_FILTER_H_

#include "../flow/node.h"
#include "../flow/node_job.h"
#include "../flow/filter_node.h"
#include "../queue/frame.h"
#include "filter_parameter.h"
#include <string>

namespace mf { namespace flow {

/// Filter which performs concrete processing, base class.
/** Concrete filters are implemented as classes derived from \ref filter, \ref source_filter or \ref sink_filter. */
class filter {
public:
	template<std::size_t Dim, typename Elem> class port;
	template<std::size_t Dim, typename Elem> class input_port;
	template<std::size_t Dim, typename Elem> class output_port;
	
private:
	flow::filter_node& node_;
		
public:
	template<std::size_t Dim, typename Elem> using input_type = input_port<Dim, Elem>;
	template<std::size_t Dim, typename Elem> using output_type = output_port<Dim, Elem>;
	template<typename Value> using parameter_type = filter_parameter<Value>;

	explicit filter(filter_node& nd) : node_(nd) { }
	
	filter(const filter&) = delete;
	filter& operator=(const filter&) = delete;
	
	filter_node& this_node() noexcept { return node_; }
	const filter_node& this_node() const noexcept { return node_; }
	
	bool reached_end() const { return node_.reached_end(); }
	
	virtual ~filter() { }

	/// Set up the filter, called prior to any frame being processed.
	virtual void setup() { }
	
	/// Prepare for processing a frame.
	virtual void pre_process(node_job&) { }
	
	/// Process a frame.
	virtual void process(node_job&) = 0;
};


/// Source filter.
/** For source nodes, has no inputs, and specifies stream properties upon construction. */
class source_filter : public filter {
public:
	explicit source_filter(filter_node& nd, bool seekable = false, time_unit stream_duration = -1) :
	filter(nd) {
		nd.define_source_stream_properties(seekable, stream_duration);
	}
};


/// Sink filter.
/** For sink nodes, has no outputs. Always connected with \ref sink_node. */
class sink_filter : public filter {
public:
	explicit sink_filter(filter_node& nd) : filter(nd) { }
};


/// Filter input and output base class.
template<std::size_t Dim, typename Elem>
class filter::port {
public:
	using elem_type = Elem;
	constexpr static std::size_t dimension = Dim;

private:
	filter& filter_;

protected:
	static frame_format default_format() { return frame_format::default_format<Elem>(); }

	explicit port(filter& filt) : filter_(filt) { }
	port(const port&) = delete;
	
public:
	const filter& this_filter() const { return filter_; } 
	filter& this_filter() { return filter_; }
};


/// Output port of filter.
template<std::size_t Dim, typename Elem>
class filter::output_port : public filter::port<Dim, Elem> {
	using base = filter::port<Dim, Elem>;
	
public:
	using frame_shape_type = ndsize<Dim>;

private:
	flow::node_output& node_output_;
	frame_shape_type frame_shape_;

public:
	explicit output_port(filter& filt) :
		base(filt),
		node_output_(filt.this_node().add_output(base::default_format())) { }

	node_output& this_node_output() { return node_output_; }
	std::ptrdiff_t index() const { return node_output_.index(); }

	void define_frame_shape(const frame_shape_type& shp) {
		frame_shape_ = shp;
		node_output_.define_frame_length(shp.product());
	}
	
	const frame_shape_type& frame_shape() const { return frame_shape_; }
};


template<std::size_t Dim, typename Elem> class filter_connector_base;


/// Input port of filter.
template<std::size_t Dim, typename Elem>
class filter::input_port : public filter::port<Dim, Elem> {
	using base = filter::port<Dim, Elem>;
	using connector_type = filter_connector_base<Dim, Elem>;
	
public:
	using frame_shape_type = ndsize<Dim>;
	
private:
	flow::node_input& node_input_;
	std::unique_ptr<connector_type> connector_;

public:
	explicit input_port(filter& filt, time_unit past_window = 0, time_unit future_window = 0) :
		base(filt),
		node_input_(filt.this_node().add_input(past_window, future_window)) { }

	node_input& this_node_input() { return node_input_; }
	std::ptrdiff_t index() const { return node_input_.index(); }

	void set_activated(bool activated) {
		node_input_.set_activated(activated);
	}
	
	bool is_activated() {
		return node_input_.is_activated();
	}
	
	template<std::size_t Output_dim, typename Output_elem>
	void connect(output_port<Output_dim, Output_elem>& output);

	const frame_shape_type& frame_shape() const;
};


}}

#include "filter.tcc"

#endif