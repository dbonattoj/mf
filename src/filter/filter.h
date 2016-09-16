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
#include "../flow/processing/processing_node.h"
#include "../flow/processing/processing_node_job.h"
#include "../queue/frame.h"
#include "filter_edge.h"
#include <vector>
#include <memory>
#include <string>

namespace mf { namespace flow {

class filter_graph;
class node_graph;
class filter_input_base;
class filter_output_base;
class multiplex_node;
template<std::size_t Output_dim, typename Output_elem> class filter_output;
template<std::size_t Input_dim, typename Input_elem> class filter_input;
class filter_parameter_base;
template<typename Value> class filter_parameter;
class filter_job;


/// Filter which performs concrete processing, base class.
/** Concrete filters are implemented as classes derived from \ref filter, \ref source_filter or \ref sink_filter. */
class filter : public processing_node_handler {			
public:
	template<std::size_t Dim, typename Elem> using input_type = filter_input<Dim, Elem>;
	template<std::size_t Dim, typename Elem> using output_type = filter_output<Dim, Elem>;
	template<typename Value> using parameter_type = filter_parameter<Value>;
	using job_type = filter_job;
	
	static const std::string default_filter_name;
	static const std::string default_filter_input_name;
	static const std::string default_filter_output_name;
	
protected:
	std::string name_;

	std::vector<filter_input_base*> inputs_;
	std::vector<filter_output_base*> outputs_;
	std::vector<filter_parameter_base*> parameters_;

	bool asynchronous_ = false;
	time_unit prefetch_duration_ = 0;
	
	processing_node* node_ = nullptr;
	multiplex_node* multiplex_node_ = nullptr;
	
	bool need_multiplex_node_() const;
	
public:
	filter() = default;
	filter(const filter&) = delete;
	filter& operator=(const filter&) = delete;
	virtual ~filter() = default;

	const std::string& name() const { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }
	
	void register_input(filter_input_base&);
	void register_output(filter_output_base&);
	void register_parameter(filter_parameter_base&);
	
	std::size_t inputs_count() const { return inputs_.size(); }
	const filter_input_base& input_at(std::ptrdiff_t i) const { return *inputs_.at(i); }
	std::size_t outputs_count() const { return outputs_.size(); }
	const filter_output_base& output_at(std::ptrdiff_t i) const { return *outputs_.at(i); }
	std::size_t parameters_count() const { return parameters_.size(); }
	const filter_parameter_base& parameter_at(std::ptrdiff_t i) const { return *parameters_.at(i); }
	
	void set_asynchonous(bool);
	bool is_asynchonous() const;
	void set_prefetch_duration(time_unit);
	time_unit prefetch_duration() const;
	
	bool was_installed() const { return (node_ != nullptr); }
	virtual void install(filter_graph&, node_graph&);
	
	void handler_setup(processing_node&) final override;
	void handler_pre_process(processing_node&, processing_node_job&) final override;
	void handler_process(processing_node&, processing_node_job&) final override;

	virtual void setup() { }
	virtual void pre_process(job_type&) { }
	virtual void process(job_type&) = 0;
	
	time_unit current_time() const;
	bool reached_end() const;
};


class sink_filter : public filter {
public:
	void install(filter_graph&, node_graph&) override;
};


class source_filter : public filter {
private:
	node_stream_timing node_stream_timing_;
	
public:
	explicit source_filter(bool seekable = false, time_unit stream_duration = -1); // TODO timing define interface

	void define_source_stream_timing(const node_stream_timing&);
	const node_stream_timing& stream_timing() const noexcept;

	void install(filter_graph&, node_graph&) override;
};


/// Output port of filter, abstract base class.
class filter_output_base {
public:
	virtual const std::string& name() const = 0;
	virtual std::size_t edges_count() const = 0;
	virtual const filter& connected_filter_at_edge(std::ptrdiff_t index) const = 0;
	virtual void install(processing_node&) = 0;
	virtual void install(processing_node&, multiplex_node&) = 0;
	virtual bool frame_shape_is_defined() const = 0;
};


/// Input port of filter, abstract base class.
class filter_input_base {
public:
	virtual const std::string& name() const = 0;
	virtual void install(processing_node&) = 0;
	virtual bool is_connected() const = 0;
	virtual const filter& connected_filter() const = 0;
	virtual const filter_output_base& connected_output() const = 0;
};


/// Output port of filter.
/** Has statically defined output frame dimension and element type. */
template<std::size_t Output_dim, typename Output_elem>
class filter_output : public filter_output_base {
public:
	constexpr static std::size_t dimension = Output_dim;
	using elem_type = Output_elem;

	using edge_base_type = filter_edge_output_base<Output_dim, Output_elem>;
	using frame_shape_type = ndsize<Output_dim>;
	using view_type = ndarray_view<Output_dim, Output_elem>;

private:
	filter& filter_;
	std::string name_;
	std::vector<edge_base_type*> edges_;
	processing_node_output_channel* node_output_channel_ = nullptr;
	
	frame_shape_type frame_shape_;	

public:
	explicit filter_output(filter&);
	
	filter& this_filter() { return filter_; }
	const filter& this_filter() const { return filter_; }

	const std::string& name() const override { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }
		
	processing_node_output_channel& this_node_output_channel()
		{ Expects(node_output_channel_ != nullptr); return *node_output_channel_; }
	const processing_node_output_channel& this_node_output_channel() const
		{ Expects(node_output_channel_ != nullptr); return *node_output_channel_; }
	std::ptrdiff_t index() const { return this_node_output_channel().index(); }
	
	std::size_t edges_count() const override { return edges_.size(); }
	const filter& connected_filter_at_edge(std::ptrdiff_t index) const override
		{ return edges_.at(index)->destination_filter(); }
	void edge_has_connected(edge_base_type&);
		
	bool was_installed() const { return (node_output_channel_ != nullptr); }
	void install(processing_node&) override;
	void install(processing_node&, multiplex_node&) override;

	void define_frame_shape(const frame_shape_type& shp);
	const frame_shape_type& frame_shape() const;
	bool frame_shape_is_defined() const override;
	
	view_type get_output_view(const frame_view& generic_view);
};


/// Input port of filter.
/** Has statically defined input frame dimension and element type. */
template<std::size_t Input_dim, typename Input_elem>
class filter_input : public filter_input_base {
public:
	constexpr static std::size_t dimension = Input_dim;
	using elem_type = Input_elem;

	using edge_base_type = filter_edge_input_base<Input_dim, Input_elem>;
	using frame_shape_type = ndsize<Input_dim>;
	using full_view_type = ndarray_timed_view<Input_dim + 1, Input_elem>;

private:
	filter& filter_;
	std::string name_;
	std::unique_ptr<edge_base_type> edge_;
	
	processing_node_input* node_input_ = nullptr;
	
	time_unit past_window_ = 0;
	time_unit future_window_ = 0;

public:
	explicit filter_input(filter&, time_unit past_window = 0, time_unit future_window = 0);

	bool is_connected() const override { return (edge_ != nullptr); }
	const filter& connected_filter() const override { return edge_->origin_filter(); }
	const filter_output_base& connected_output() const override { return edge_->origin(); }

	filter& this_filter() { return filter_; }
	const filter& this_filter() const { return filter_; }

	const std::string& name() const override { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }

	processing_node_input& this_node_input() { Expects(node_input_ != nullptr); return *node_input_; }
	const processing_node_input& this_node_input() const { Expects(node_input_ != nullptr); return *node_input_; }
	std::ptrdiff_t index() const { return this_node_input().index(); }
	
	template<std::size_t Output_dim, typename Output_elem>
	void connect(filter_output<Output_dim, Output_elem>&);
	
	template<std::size_t Output_dim, typename Output_elem, typename Convert_function>
	void connect(filter_output<Output_dim, Output_elem>&, Convert_function&&);

	template<typename Casted_elem, std::size_t Output_dim, typename Output_elem, typename Convert_function>
	void connect(filter_output<Output_dim, Output_elem>&, Convert_function&&);
	
	const frame_shape_type& frame_shape() const;
	
	bool was_installed() const { return (node_input_ != nullptr); }
	void install(processing_node&) override;
	
	void set_activated(bool);
	bool is_activated();
	
	void activate() { set_activated(true); }
	void deactivate() { set_activated(false); }
	
	full_view_type get_input_view(const timed_frame_array_view& generic_view);
};

}}

#include "filter.tcc"

#endif
