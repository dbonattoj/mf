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

#ifndef MF_FLOW_GRAPH_H_
#define MF_FLOW_GRAPH_H_

#include "../common.h"
#include "../os/event.h"
#include "filter_node.h"
#include "sink_node.h"
#include "../filter/convert_filter.h"
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <functional>

namespace mf { namespace flow {

class node;
class sync_node;
class filter;
class sink_filter;

/// Graph containing interconnected nodes through which media frames flow.
class graph {
public:
	using frame_callback_function_type = void(time_unit t);
	
private:
	std::vector<std::unique_ptr<node>> nodes_;
	sink_node* sink_ = nullptr;
	bool was_setup_ = false;
	bool running_ = false;
	sticky_event stop_event_;
	
public:
	template<typename Node, typename... Args>
	Node& add_node_(Args&&... args) {
		static_assert(std::is_base_of<node, Node>::value, "");
		if(was_setup_) throw std::logic_error("cannot add node after graph already set up");
		Node* nd = new Node(*this, std::forward<Args>(args)...);
		nodes_.emplace_back(nd);
		return *nd;
	}
	
	template<typename Node, typename... Args>
	Node& add_sink_(Args&&... args) {
		static_assert(std::is_base_of<sink_node, Node>::value, "");
		Node& sink = add_node_<Node>(std::forward<Args>(args)...);
		sink_ = &sink;
		return sink;
	}
	
	void pull_next_frame_();


public:
	std::function<frame_callback_function_type> callback_function;

	~graph();
	
	template<typename Filter, typename Node = sync_node, typename... Args>
	Filter& add_filter(Args&&... args) {
		static_assert(std::is_base_of<filter, Filter>::value, "");
		static_assert(std::is_base_of<filter_node, Node>::value, "");
		filter_node& nd = add_node_<Node>();
		return nd.set_filter<Filter>(std::forward<Args>(args)...);
	}
	
	template<std::size_t Dim, typename Input_elem, typename Output_elem, typename Element_function>
	auto& add_convert_filter(Element_function func) {
		using filter_type = convert_filter<Dim, Input_elem, Output_elem, Element_function>;
		return add_filter<filter_type>(func);
	}
		
	template<typename Filter, typename... Args>
	Filter& add_sink_filter(Args&&... args) {
		static_assert(std::is_base_of<sink_filter, Filter>::value, "");
		filter_node& nd = add_sink_<sink_node>();
		return nd.set_filter<Filter>(std::forward<Args>(args)...);
	}
	
	bool was_setup() const { return was_setup_; }
	bool is_running() const { return running_; }
	
	event& stop_event() { return stop_event_; }
	
	void setup();
	
	void launch();
	void stop();

	time_unit current_time() const;
	
	void run_until(time_unit last_frame);
	void run_for(time_unit duration);
	bool run();

	void seek(time_unit target_time);
};

}}

#endif
