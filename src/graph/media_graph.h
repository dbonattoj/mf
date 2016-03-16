#ifndef MF_MEDIA_GRAPH_H_
#define MF_MEDIA_GRAPH_H_

#include <vector>
#include <memory>
#include <utility>
#include <stdexcept>
#include <type_traits>
#include "../common.h"

namespace mf {

class media_node;
class media_sink_node;

class media_graph {
private:
	std::vector<std::unique_ptr<media_node>> nodes_; ///< Nodes in the graph, including sink.
	media_sink_node* sink_ = nullptr; ///< Sink node.
	bool setup_ = false; ///< True after setup() was called.

public:
	~media_graph();

	template<typename Node, typename... Args>
	Node& add_node(Args&&... args) {
		static_assert(std::is_base_of<media_node, Node>::value, "sink node must be subclass of media_node");
		if(setup_) throw std::logic_error("cannot add node after graph already set up");
		Node* node = new Node(std::forward<Args>(args)...);
		nodes_.emplace_back(node);
		return *node;
	}
	
	template<typename Node, typename... Args>
	Node& add_sink(Args&&... args) {
		static_assert(std::is_base_of<media_sink_node, Node>::value, "sink node must be subclass of media_sink_node");
		Node* sink = add_node(std::forward<Args>(args)...);
		sink_ = &sink;
		return *sink;
	}
	
	void setup();
	
	time_unit current_time() const;
	
	void run_until(time_unit last_frame);
	void run();
};

}

#endif
