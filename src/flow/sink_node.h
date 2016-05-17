#ifndef MF_FLOW_SINK_NODE_H_
#define MF_FLOW_SINK_NODE_H_

#include "node.h"
#include "node_job.h"
#include "node_parameter.h"

namespace mf { namespace flow {

class graph;

/// Sink node base class.
/** Has one of multiple inputs and no outputs. There is one sink node per graph. Controls time flow of graph. */
class sink_node final : public filter_node {
public:	
	template<typename Value>
	using parameter_type = node_parameter<Value>;

	explicit sink_node(graph& gr) : filter_node(gr) { }
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
	bool process_next_frame() final override;
	
	void setup_graph();
	
	void pull(time_unit t);
	void pull_next_frame() { process_next_frame(); }
	
	void seek(time_unit t);
	
	node_input& add_input(time_unit past_window, time_unit future_window) override {
		return add_input_<node_input>(past_window, future_window);
	}
	
	node_output& add_output(const frame_format& format) override { throw 0; }
};


}}

#endif
