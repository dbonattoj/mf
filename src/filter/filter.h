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
#include "../flow/filter_node.h"
#include "../queue/frame.h"
#include "filter_edge.h"
#include "filter_parameter.h"
#include "filter_job.h"
#include <vector>
#include <memory>

namespace mf { namespace flow {

class graph;
class filter_input_base;
class filter_output_base;
class multiplex_node;
template<std::size_t Output_dim, typename Output_elem> class filter_output;
template<std::size_t Input_dim, typename Input_elem> class filter_input;

/// Filter which performs concrete processing, base class.
/** Concrete filters are implemented as classes derived from \ref filter, \ref source_filter or \ref sink_filter. */
class filter {			
public:
	template<std::size_t Dim, typename Elem> using input_type = filter_input<Dim, Elem>;
	template<std::size_t Dim, typename Elem> using output_type = filter_output<Dim, Elem>;
	template<typename Value> using parameter_type = filter_parameter<Value>;
	using job_type = filter_job;

protected:
	std::vector<filter_input_base*> inputs_;
	std::vector<filter_output_base*> outputs_;

	bool asynchronous_ = false;
	time_unit prefetch_duration_ = 0;
	
	filter_node* node_ = nullptr;
	
public:
	filter() = default;
	filter(const filter&) = delete;
	filter& operator=(const filter&) = delete;
	virtual ~filter() = default;
	
	void register_input(filter_input_base&);
	void register_output(filter_output_base&);
	
	void set_asynchonous(bool);
	bool is_asynchonous() const;
	void set_prefetch_duration(time_unit);
	time_unit prefetch_duration() const;
	
	bool was_installed() const { return (node_ != nullptr); }
	virtual void install(graph&);

	virtual void setup() { }
	virtual void pre_process(job_type&) { }
	virtual void process(job_type&) = 0;
	
	time_unit current_time() const;
	bool reached_end() const;
};


class sink_filter : public filter {
public:
	void install(graph&) override;
};


class source_filter : public filter {
private:
	node_stream_properties node_stream_properties_;
	
public:
	explicit source_filter(bool seekable = false, time_unit stream_duration = -1);

	void define_source_stream_properties(const node_stream_properties&);
	const node_stream_properties& stream_properties() const noexcept;

	void install(graph&) override;
};


class filter_output_base {
public:
	virtual void install(filter_node&) = 0;
};



class filter_input_base {
public:
	virtual void install(filter_node&) = 0;
};


template<std::size_t Output_dim, typename Output_elem>
class filter_output : public filter_output_base {
public:
	constexpr static std::size_t dimension = Output_dim;
	using elem_type = Output_elem;

	using edge_base_type = filter_edge_output_base<Output_dim, Output_elem>;
	using frame_shape_type = ndsize<Output_dim>;

private:
	std::vector<edge_base_type*> edges_;
		
	filter_node_output* node_output_ = nullptr;
	std::unique_ptr<multiplex_node> multiplex_node_;
	
	frame_shape_type frame_shape_;	

public:
	explicit filter_output(filter&);

	filter_node_output& this_node_output() { Expects(node_output_ != nullptr); return *node_output_; }
	const filter_node_output& this_node_output() const { Expects(node_output_ != nullptr); return *node_output_; }
	std::ptrdiff_t index() const { return this_node_output().index(); }
	
	void edge_has_connected(edge_base_type&);
		
	bool was_installed() const { return (node_output_ != nullptr); }
	void install(filter_node&) override;

	void define_frame_shape(const frame_shape_type& shp);
	const frame_shape_type& frame_shape() const;
};


template<std::size_t Input_dim, typename Input_elem>
class filter_input : public filter_input_base {
public:
	constexpr static std::size_t dimension = Input_dim;
	using elem_type = Input_elem;

	using edge_base_type = filter_edge_input_base<Input_dim, Input_elem>;
	using frame_shape_type = ndsize<Input_dim>;

private:
	std::unique_ptr<edge_base_type> edge_;
	
	node_input* node_input_ = nullptr;
	
	time_unit past_window_ = 0;
	time_unit future_window_ = 0;

public:
	explicit filter_input(filter&, time_unit past_window = 0, time_unit future_window = 0);

	node_input& this_node_input() { Expects(node_input_ != nullptr); return *node_input_; }
	const node_input& this_node_input() const { Expects(node_input_ != nullptr); return *node_input_; }
	std::ptrdiff_t index() const { return this_node_input().index(); }
	
	template<std::size_t Output_dim, typename Output_elem>
	void connect(filter_output<Output_dim, Output_elem>&);
	
	const frame_shape_type& frame_shape() const;
	
	bool was_installed() const { return (node_input_ != nullptr); }
	void install(filter_node&) override;
	
	void set_activated(bool);
	bool is_activated();
	
	void activate() { set_activated(true); }
	void deactivate() { set_activated(false); }
};

}}

#include "filter.tcc"

#endif
