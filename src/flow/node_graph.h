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

#ifndef MF_FLOW_NODE_GRAPH_H_
#define MF_FLOW_NODE_GRAPH_H_

#include "../common.h"
#include "diagnostic/diagnostic_handler.h"
#include "node.h"
#include "processing/sink_node.h"
#include "types.h"
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <functional>

namespace mf { namespace flow {

class node;

/// Graph containing interconnected nodes through which media frames flow.
/** Low-level graph composed of \ref node objects. High-level graph of \ref filter obbjects is \ref filter_graph. */
class node_graph {
public:
	using frame_callback_function_type = void(time_unit t);
	
private:
	std::vector<std::unique_ptr<node>> nodes_;
	sink_node* sink_ = nullptr;
	bool was_setup_ = false;
	bool launched_ = false;
	
	thread_index last_thread_index_ = 0;
	
	std::atomic<bool> was_stopped_ {false};
	
	diagnostic_handler* diagnostic_handler_ = nullptr;

	node::pull_result pull_next_frame_();

public:
	std::function<frame_callback_function_type> callback_function;

	node_graph() = default;
	~node_graph();
	
	template<typename Node, typename... Args>
	Node& add_node(Args&&... args) {
		static_assert(std::is_base_of<node, Node>::value, "node must be derived class from `node`");
		Expects(! was_setup_);
		Node* nd = new Node(*this, std::forward<Args>(args)...);
		nodes_.emplace_back(nd);
		return *nd;
	}
	
	template<typename Node, typename... Args>
	Node& add_sink(Args&&... args) {
		static_assert(std::is_base_of<sink_node, Node>::value, "sink node must be derived class from `sink_node`");
		Expects(! was_setup_);
		Node& sink = add_node<Node>(std::forward<Args>(args)...);
		sink_ = &sink;
		return sink;
	}
	
	thread_index new_thread_index();
	thread_index root_thread_index() const;
	bool was_stopped() const { return was_stopped_; }
	
	void set_diagnostic(diagnostic_handler& handler) { diagnostic_handler_ = &handler; }
	void unset_diagnostic() { diagnostic_handler_ = nullptr; }
	bool has_diagnostic() const { return (diagnostic_handler_ != nullptr); }
	diagnostic_handler& diagnostic() { Assert(has_diagnostic()); return *diagnostic_handler_; }
	const diagnostic_handler& diagnostic() const { Assert(has_diagnostic()); return *diagnostic_handler_; }
	
	std::size_t nodes_count() const { return nodes_.size(); }
	const node& node_at(std::ptrdiff_t i) const { return *nodes_.at(i); }
	node& node_at(std::ptrdiff_t i) { return *nodes_.at(i); }
	sink_node& sink() { return *sink_; }
	const sink_node& sink() const { return *sink_; }
	
	bool was_setup() const { return was_setup_; }
	bool is_launched() const { return launched_; }
	
		
	void setup();
	
	void launch();
	void stop();

	time_unit current_time() const;
	
	void run_until(time_unit last_frame);
	void run_for(time_unit duration);
	void run();

	void seek(time_unit target_time);
};

}}

#endif
