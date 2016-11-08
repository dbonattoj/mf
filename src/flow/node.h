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
#include "timing/stream_timing.h"

#include <vector>
#include <map>
#include <atomic>
#include <string>
#include <memory>
#include <utility>
#include <type_traits>
#include <mutex>

#define NodeDebug(...) MF_DEBUG_T("node", "node '", name(), "' t=", current_time(), ":\n", __VA_ARGS__)

namespace mf { namespace flow {

class node_graph;
class node_output;
class node_input;

/// Node in flow graph, base class.
class node {
public:
	/// Result state of pulling a span of frames from the node.
	enum class pull_result {
		undefined,
		success, ///< The full span was successfully pulled.
		fatal_failure, ///< A failure occured which will cause graph execution to stop.
		transitory_failure, ///< A failure occured which will be handled within graph.
		stopped, ///< The node graph execution was stopped.
		end_of_stream ///< The full span was not pulled because it crossed the end of stream.
	};

private:
	enum class stage { construction, was_pre_setup, was_setup };
	using sent_parameter_relay_type = std::function<void(parameter_id, const node_parameter_value&)>;
	
	stage stage_ = stage::construction;

	node_graph& graph_;
	std::vector<std::unique_ptr<node_output>> outputs_;
	std::vector<std::unique_ptr<node_input>> inputs_;
	
	/// Timing of the frames pulled out of this node.
	/** Defines correspondence of frame index (called "time") and real clock time.  */
	stream_timing output_timing_;
		
	/// Parameters of preceding nodes whose values this node receives with input frames.
	/** Propagated parameters on node outputs are set up such that the node receives these parameters. */
	std::vector<parameter_id> input_parameters_;
	
	/// Parameters of preceding nodes to which this node can send a new value.
	/** Parameter relays between owning node and this node are set up to transfer the new value to the owning node. */
	std::vector<parameter_id> sent_parameters_;
	std::map<parameter_id, sent_parameter_relay_type> sent_parameter_relays_;
	
	std::string name_ = "node";


	/// Dynamic state (varies during execution).
	///@{
	std::atomic<time_unit> current_time_ {-1};
	///@}

	
	/// Recursively pre-setup nodes in sink-to-source order.
	/** Must be called on sink node. Calls pre_setup() once on each node in graph, in an order such that when one node
	 ** is pre-setup, its successors have already been pre-setup. */
	void propagate_pre_setup_();
	
	/// Recursively setup nodes in source-to-sink order.
	/** Must be called on sink node. Calls setup() once on each node in graph, in an order such that when one node
	 ** is setup, its predecessors have already been setup */
	void propagate_setup_();
		

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
		
	void set_current_time_(time_unit t) ;
	void mark_end_();
		
public:
	virtual ~node();

	const node_graph& graph() const { return graph_; }
	node_graph& graph() { return graph_; }
	
	bool was_setup() const { return (stage_ == stage::was_setup); }

	const auto& inputs() const { return inputs_; }
	const auto& outputs() const { return outputs_; }

	std::size_t inputs_count() const { return inputs_.size(); }
	node_input& input_at(std::ptrdiff_t index) { return *inputs_.at(index); }
	const node_input& input_at(std::ptrdiff_t index) const { return *inputs_.at(index); }
	std::size_t outputs_count() const { return outputs_.size(); }
	node_output& output_at(std::ptrdiff_t index) { return *outputs_.at(index); }
	const node_output& output_at(std::ptrdiff_t index) const { return *outputs_.at(index); }
	
	bool is_source() const { return inputs_.empty(); }
	bool is_sink() const { return outputs_.empty(); }


	/// Input parameters.
	///@{
	void add_input_parameter(parameter_id);
	bool has_input_parameter(parameter_id) const;
	std::size_t input_parameters_count() const { return input_parameters_.size(); }
	parameter_id input_parameter_at(std::ptrdiff_t i) const { return input_parameters_.at(i); }
	
	bool add_propagated_parameter_if_needed(parameter_id);
	///@}
	
	
	/// Sent parameters.
	///@{
	void add_sent_parameter(parameter_id);
	bool has_sent_parameter(parameter_id) const;
	std::size_t sent_parameters_count() const { return sent_parameters_.size(); }
	parameter_id sent_parameter_at(std::ptrdiff_t i) const { return sent_parameters_.at(i); }
		
	bool add_relayed_parameter_if_needed(parameter_id, const sent_parameter_relay_type& preceding_relay);
	sent_parameter_relay_type sent_parameter_relay(parameter_id) const;
	virtual sent_parameter_relay_type custom_sent_parameter_relay_(parameter_id, const sent_parameter_relay_type& def);
	///@}
	
	
	/// Graph algorithms.
	///@{
	bool precedes(const node&) const;
	bool precedes_strict(const node&) const;
	const node& first_successor() const;
	///@}
	
	
	/// Node-specific.
	///@{	
	virtual time_unit minimal_offset_to(const node&) const = 0;
	virtual time_unit maximal_offset_to(const node&) const = 0;

	virtual void pre_setup() { }
	virtual void setup() { }
	virtual void launch() { }
	virtual void pre_stop() { }
	virtual void stop() { }
	///@}
	
	
	void define_output_timing(const stream_timing& tm) { output_timing_ = tm; }
	const stream_timing& output_timing() const { return output_timing_; }
	
	const std::string& name() const { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }
	
	void setup_sink();

	time_unit current_time() const { return current_time_; }
};


}}

#endif
