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
#include <map>

namespace mf { namespace flow {

class filter_graph;
class filter_input_base;
class filter_output_base;
class filter_parameter_base;
class filter_job;
class filter_handler;

class node_graph;
class processing_node;
class multiplex_node;


/// Filter which performs concrete processing.
class filter : public processing_node_handler {			
public:	
	static const std::string default_filter_name;
	static const std::string default_filter_input_name;
	static const std::string default_filter_output_name;

	struct filter_node_group {
		processing_node* processing = nullptr;
		multiplex_node* multiplex = nullptr;
	};

	struct installation_guide {
		filter_graph& filter_gr;
		node_graph& node_gr;
		std::map<const filter*, filter_node_group> local_filter_nodes;
				
		installation_guide(filter_graph& fg, node_graph& ng) :
			filter_gr(fg), node_gr(ng) { }
		
		bool has_filter(const filter&) const;
		bool has_filter_successors(const filter&) const;
		bool has_filter_predecessors(const filter&) const;
	};

private:
	std::unique_ptr<filter_handler> handler_;
	
	std::string name_;

	std::vector<filter_input_base*> inputs_;
	std::vector<filter_output_base*> outputs_;
	std::vector<filter_parameter_base*> parameters_;

	bool asynchronous_ = false;
	time_unit prefetch_duration_ = 0;
	//int parallelization_factor_ = 1; // TODO
	
	bool was_setup_ = false;
	
	bool needs_multiplex_node_() const;
	//bool is_parallelization_join_point_() const;
	//bool is_parallelization_split_point_() const;

	void setup_();
	void install_(installation_guide&);
	void install_input_(filter_input_base&, processing_node&, const installation_guide&);

	using filter_reference_set = std::set<filter*>;
	using const_filter_reference_set = std::set<const filter*>;
	filter_reference_set direct_successors_();
	const_filter_reference_set direct_successors_() const;
	filter_reference_set direct_predecessors_();
	const_filter_reference_set direct_predecessors_() const;


public:
	explicit filter(std::unique_ptr<filter_handler>);
	explicit filter() { }
	
	void set_handler(std::unique_ptr<filter_handler> hnd) { handler_ = std::move(hnd); }
	
	filter_handler& handler() { return *handler_; }
	const filter_handler& handler() const { return *handler_; }

	bool precedes(const filter&) const;
	bool precedes_strict(const filter&) const;

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
	
	bool is_sink() const { return (outputs_count() == 0); }
	bool is_source() const { return (inputs_count() == 0); }
	
	void set_asynchonous(bool);
	bool is_asynchonous() const;
	void set_prefetch_duration(time_unit);
	time_unit prefetch_duration() const;
	
	//int parallelization_factor() const { return parallelization_factor_; } 
	
	void propagate_setup();
	void propagate_install(installation_guide&);
	
	void handler_pre_process(processing_node&, processing_node_job&) final override;
	void handler_process(processing_node&, processing_node_job&) final override;

	node_stream_timing node_stream_timing_;
	void define_source_stream_timing(const node_stream_timing& tm) { node_stream_timing_ = tm; }
};




template<typename Handler>
class filter_derived : public filter {
private:
	Handler* handler_;

public:
	template<typename... Args>
	explicit filter_derived(Args&&... args) :
		handler_(new Handler(*this, std::forward<Args>(args)...))
	{
		set_handler(std::unique_ptr<Handler>(handler_));
	}
			
	Handler& handler() { return handler_; }
	const Handler& handler() const { return handler_; }
	
	Handler& operator*() { return *handler_; }
	const Handler& operator*() const { return *handler_; }
	Handler* operator->() { return handler_; }
	const Handler* operator->() const { return handler_; }
};



/// Output port of filter, abstract base class.
class filter_output_base {
public:
	virtual const std::string& name() const = 0;
	virtual std::size_t edges_count() const = 0;
	virtual filter& connected_filter_at_edge(std::ptrdiff_t index) const = 0;
	
	virtual ndarray_format frame_format() const = 0;
	virtual bool frame_shape_is_defined() const = 0;

	virtual void set_index(std::ptrdiff_t idx) = 0;
	virtual std::ptrdiff_t index() const = 0;
};



/// Input port of filter, abstract base class.
class filter_input_base {
public:
	virtual const std::string& name() const = 0;
	virtual bool is_connected() const = 0;
	virtual filter& connected_filter() const = 0;
	virtual filter_output_base& connected_output() const = 0;
	
	virtual time_unit past_window_duration() const = 0;
	virtual time_unit future_window_duration() const = 0;
	
	virtual void install_edge(node_output& origin_node_output, std::ptrdiff_t origin_node_channel_index, node_input& destination_node_input) = 0;
	virtual void set_index(std::ptrdiff_t idx) = 0;
	virtual std::ptrdiff_t index() const = 0;
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
	
	std::ptrdiff_t index_ = -1;
	
	frame_shape_type frame_shape_;	

public:
	explicit filter_output(filter&, const std::string& name = "");
	
	filter& this_filter() { return filter_; }
	const filter& this_filter() const { return filter_; }

	const std::string& name() const override { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }
			
	std::size_t edges_count() const override { return edges_.size(); }
	filter& connected_filter_at_edge(std::ptrdiff_t index) const override
		{ return edges_.at(index)->destination_filter(); }
	void edge_has_connected(edge_base_type&);
		
	void set_index(std::ptrdiff_t idx) override { index_ = idx; }
	std::ptrdiff_t index() const override { return index_; }	
		
	void define_frame_shape(const frame_shape_type& shp);
	const frame_shape_type& frame_shape() const;
	bool frame_shape_is_defined() const override;
	ndarray_format frame_format() const override;
	
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
	
	std::ptrdiff_t index_ = -1;
	
	time_unit past_window_ = 0;
	time_unit future_window_ = 0;

public:
	explicit filter_input(filter&, const std::string& name = "");

	filter& this_filter() { return filter_; }
	const filter& this_filter() const { return filter_; }

	const std::string& name() const override { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }

	void set_past_window(time_unit dur) { past_window_ = dur; }
	void set_future_window(time_unit dur) { future_window_ = dur; }
	time_unit past_window_duration() const override { return past_window_; }
	time_unit future_window_duration() const override  { return future_window_; }

	bool is_connected() const override { return (edge_ != nullptr); }
	filter& connected_filter() const override { return edge_->origin_filter(); }
	filter_output_base& connected_output() const override { return edge_->origin(); }
	
	template<std::size_t Output_dim, typename Output_elem>
	void connect(filter_output<Output_dim, Output_elem>&);
	
	template<std::size_t Output_dim, typename Output_elem, typename Convert_function>
	void connect(filter_output<Output_dim, Output_elem>&, Convert_function&&);

	template<typename Casted_elem, std::size_t Output_dim, typename Output_elem, typename Convert_function>
	void connect(filter_output<Output_dim, Output_elem>&, Convert_function&&);
	
	const frame_shape_type& frame_shape() const;
	
	void install_edge(node_output& origin_node_output, std::ptrdiff_t origin_node_channel_index, node_input& destination_node_input) override;
	void set_index(std::ptrdiff_t idx) override { index_ = idx; }
	std::ptrdiff_t index() const override { return index_; }	

	full_view_type get_input_view(const timed_frame_array_view& generic_view);
};

}}

#include "filter.tcc"

#endif
