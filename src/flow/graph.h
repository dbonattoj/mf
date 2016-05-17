#ifndef MF_FLOW_GRAPH_H_
#define MF_FLOW_GRAPH_H_

#include "../common.h"
#include "../os/event.h"
#include "filter_node.h"
#include "sink_node.h"
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace mf { namespace flow {

class node;
class sync_node;
class filter;
class sink_filter;

/// Graph containing interconnected nodes through which media frames flow.
class graph {
private:
	std::vector<std::unique_ptr<node>> nodes_;
	sink_node* sink_ = nullptr;
	bool was_setup_ = false;
	bool running_ = false;
	sticky_event stop_event_;
	
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


public:
	~graph();
	
	template<typename Filter, typename Node = sync_node, typename... Args>
	Filter& add_filter(Args&&... args) {
		static_assert(std::is_base_of<filter, Filter>::value, "");
		static_assert(std::is_base_of<filter_node, Node>::value, "");
		filter_node& nd = add_node_<Node>();
		return nd.set_filter<Filter>(std::forward<Args>(args)...);
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
