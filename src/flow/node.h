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

#ifndef MF_FLOW_NODE_H_
#define MF_FLOW_NODE_H_

#include "../common.h"
#include "../queue/frame.h"
#include "parameter/node_parameter.h"
#include "node_stream_properties.h"

#include <vector>
#include <map>
#include <atomic>
#include <string>
#include <memory>
#include <utility>
#include <type_traits>

namespace mf { namespace flow {

class graph;
class node_output;
class node_input;

using thread_index = int;
static constexpr thread_index undefined_thread_index = -1;

/// Node in flow graph, base class.
class node {
public:
	enum online_state { online, offline, reconnecting };
	enum pull_result { success, transitory_failure, stopped };

private:
	enum class stage { construction, was_pre_setup, was_setup };
	
	stage stage_ = stage::construction;

	graph& graph_;
	std::vector<std::unique_ptr<node_output>> outputs_;
	std::vector<std::unique_ptr<node_input>> inputs_;	
	node_stream_properties stream_properties_;
		
	std::atomic<online_state> state_ {online};
	std::atomic<time_unit> current_time_ {-1};
	std::atomic<bool> reached_end_ {false};
	
	std::map<parameter_id, node_parameter> parameters_;
	std::vector<parameter_id> input_parameters_;
	
	std::string name_ = "node";
	
	/// Recursively pre-setup nodes in sink-to-source order.
	/** Must be called on sink node. Calls pre_setup() once on each node in graph, in an order such that when one node
	 ** is pre-setup, its successors have already been pre-setup. */
	void propagate_pre_setup_();
	
	/// Recursively setup nodes in source-to-sink order.
	/** Must be called on sink node. Calls setup() once on each node in graph, in an order such that when one node
	 ** is setup, its predecessors have already been setup */
	void propagate_setup_();
	
	
	void deduce_stream_properties_();


	void deduce_output_parameters_();
	

protected:
	explicit node(graph& gr) : graph_(gr) { }
	node(const node&) = delete;
	node& operator=(const node&) = delete;
	
	void verify_connections_validity_() const;

	void add_input_ptr_(std::unique_ptr<node_input>&& in) {
		inputs_.push_back(std::move(in));
	}
	
	void add_output_ptr_(std::unique_ptr<node_output>&& out) {
		outputs_.push_back(std::move(out));
	}
		
	void set_current_time_(time_unit t) noexcept;
	void mark_end_();
	
public:
	virtual ~node();

	const graph& this_graph() const noexcept { return graph_; }
	graph& this_graph() noexcept { return graph_; }
	
	bool was_setup() const { return (stage_ == stage::was_setup); }

	const auto& inputs() const noexcept { return inputs_; }
	const auto& outputs() const noexcept { return outputs_; }

	std::size_t inputs_count() const noexcept { return inputs_.size(); }
	node_input& input_at(std::ptrdiff_t index) { return *inputs_.at(index); }
	const node_input& input_at(std::ptrdiff_t index) const { return *inputs_.at(index); }
	std::size_t outputs_count() const noexcept { return outputs_.size(); }
	node_output& output_at(std::ptrdiff_t index) { return *outputs_.at(index); }
	const node_output& output_at(std::ptrdiff_t index) const { return *outputs_.at(index); }
	
	bool is_source() const noexcept { return inputs_.empty(); }
	bool is_sink() const noexcept { return outputs_.empty(); }
	
	node_parameter& add_parameter(parameter_id);
	bool has_parameter(parameter_id) const;
	node_parameter& parameter_at(parameter_id);
	const node_parameter& parameter_at(parameter_id) const;
	void add_input_parameter(parameter_id);
	bool has_input_parameter(parameter_id) const;
	bool needs_output_parameter(parameter_id) const;

	bool precedes(const node&) const;
	bool precedes_strict(const node&) const;
	const node& first_successor() const;
			
	virtual time_unit minimal_offset_to(const node&) const = 0;
	virtual time_unit maximal_offset_to(const node&) const = 0;
	
	void define_source_stream_properties(const node_stream_properties&);
	const node_stream_properties& stream_properties() const noexcept { return stream_properties_; }
	
	const std::string& name() const { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }
	
	void setup_sink();

	online_state state() const { return state_; }
	void propagate_offline_state();
	void propagate_reconnecting_state();
	void set_online();
			
	virtual void pre_setup() { }
	virtual void setup() { }
	virtual void launch() { }
	virtual void pre_stop() { }
	virtual void stop() { }

	bool is_bounded() const;
	time_unit current_time() const noexcept { return current_time_; }
	bool reached_end() const noexcept { return reached_end_; }
	time_unit end_time() const noexcept;
};


}}

#endif
