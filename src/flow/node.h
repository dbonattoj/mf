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

#include "types.h"
#include "../common.h"
#include "../queue/frame.h"
#include "frame/node_frame_view.h"
#include "parameter/node_parameter.h"
#include "parameter/node_parameter_value.h"
#include "parameter/node_parameter_valuation.h"
#include "parameter/node_parameter_relay.h"
#include "node_stream_properties.h"

#include <vector>
#include <map>
#include <atomic>
#include <string>
#include <memory>
#include <utility>
#include <type_traits>
#include <mutex>

namespace mf { namespace flow {

class node_graph;
class node_output;
class node_input;

/// Node in flow graph, base class.
class node {
public:
	enum online_state { online, offline, reconnecting };
	enum pull_result { success, transitory_failure, stopped };

private:
	enum class stage { construction, was_pre_setup, was_setup };
	
	stage stage_ = stage::construction;

	node_graph& graph_;
	std::vector<std::unique_ptr<node_output>> outputs_;
	std::vector<std::unique_ptr<node_input>> inputs_;	
	node_stream_properties stream_properties_;			

	/// Parameters owned by this node.
	/** Values are stored in `parameter_valuation_`. */
	std::vector<node_parameter> parameters_;
	
	/// Parameters of preceding nodes whose values this node receives with input frames.
	/** Propagated parameters on node outputs are set up such that the node receives these parameters. */
	std::vector<node_parameter_id> input_parameters_;
	
	/// Parameters of preceding nodes to which this node can send a new value.
	/** Parameter relays between owning node and this node are set up to transfer the new value to the owning node. */
	std::vector<node_parameter_id> sent_parameters_;
	
	/// Relay for sent parameter values from suceeding node to this or preceding node.
	node_parameter_relay sent_parameters_relay_;
	
	std::string name_ = "node";


	/// Dynamic state (varies during execution).
	///@{
	std::atomic<online_state> state_ {online};
	std::atomic<time_unit> current_time_ {-1};
	std::atomic<bool> reached_end_ {false};

	node_parameter_valuation parameter_valuation_; ///< Current valuation of node parameters.
	mutable std::mutex parameters_mutex_; ///< Mutex to protect parameter_valuation_ during concurrent access.
	///@}


	
	/// Recursively pre-setup nodes in sink-to-source order.
	/** Must be called on sink node. Calls pre_setup() once on each node in graph, in an order such that when one node
	 ** is pre-setup, its successors have already been pre-setup. */
	void propagate_pre_setup_();
	
	/// Recursively setup nodes in source-to-sink order.
	/** Must be called on sink node. Calls setup() once on each node in graph, in an order such that when one node
	 ** is setup, its predecessors have already been setup */
	void propagate_setup_();
	
	
	void deduce_stream_properties_();


	void deduce_propagated_parameters_();
	
	
	void deduce_sent_parameters_relay_();
	

protected:
	explicit node(node_graph& gr) : graph_(gr) { }
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
	
	void update_parameter_(node_parameter_id, const node_parameter_value&);
	void update_parameter_(node_parameter_id, node_parameter_value&&);
	void update_parameters_(const node_parameter_valuation&);
	void update_parameters_(node_parameter_valuation&&);
	node_parameter_valuation current_parameter_valuation_() const;
	
public:
	virtual ~node();

	const node_graph& graph() const noexcept { return graph_; }
	node_graph& graph() noexcept { return graph_; }
	
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
	
	/// Owned parameters.
	///@{
	node_parameter& add_parameter(node_parameter_id, const node_parameter_value& initial_value);
	bool has_parameter(node_parameter_id) const;
	std::size_t parameters_count() const { return parameters_.size(); }
	const node_parameter& parameter_at(std::ptrdiff_t i) const { return parameters_.at(i); }
	///@}
	
	/// Input parameters.
	///@{
	void add_input_parameter(node_parameter_id);
	bool has_input_parameter(node_parameter_id) const;
	std::size_t input_parameters_count() const { return input_parameters_.size(); }
	node_parameter_id input_parameter_at(std::ptrdiff_t i) const { return input_parameters_.at(i); }
	///@}
	
	/// Output parameters.
	///@{
	void add_sent_parameter(node_parameter_id);
	bool has_sent_parameter(node_parameter_id) const;
	std::size_t sent_parameters_count() const { return sent_parameters_.size(); }
	node_parameter_id sent_parameter_at(std::ptrdiff_t i) const { return sent_parameters_.at(i); }
	///@}
	
	bool add_propagated_parameter_if_needed(node_parameter_id);
	bool add_relayed_parameter_if_needed(node_parameter_id);
	
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
