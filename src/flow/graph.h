#ifndef MF_FLOW_GRAPH_H_
#define MF_FLOW_GRAPH_H_

#include "../common.h"
#include "../utility/event.h"
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace mf { namespace flow {

class node;
class sink_node;

/// Graph containing interconnected nodes through which media frames flow.
class graph {
private:
	std::vector<std::unique_ptr<node>> nodes_;
	sink_node* sink_ = nullptr;
	bool was_setup_ = false;
	bool running_ = false;
	sticky_event stop_event_;

public:
	~graph();

	template<typename Node, typename... Args>
	Node& add_node(Args&&... args) {
		static_assert(std::is_base_of<node, Node>::value, "sink node must be subclass of media_node");
		if(was_setup_) throw std::logic_error("cannot add node after graph already set up");
		Node* nd = new Node(*this, std::forward<Args>(args)...);
		nodes_.emplace_back(nd);
		return *nd;
	}
	
	template<typename Node, typename... Args>
	Node& add_sink(Args&&... args) {
		static_assert(std::is_base_of<sink_node, Node>::value, "sink node must be subclass of media_sink_node");
		Node& sink = add_node<Node>(std::forward<Args>(args)...);
		sink_ = &sink;
		return sink;
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
	void run();

	void seek(time_unit target_time);
};

}}

#endif
