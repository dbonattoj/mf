#ifndef MF_FLOW_SINK_NODE_H_
#define MF_FLOW_SINK_NODE_H_

#include "node.h"
#include "node_job.h"
#include "node_io_wrapper.h"
#include "node_parameter.h"

namespace mf { namespace flow {

class graph;

/// Sink node base class.
/** Has one of multiple inputs and no outputs. There is one sink node per graph. Controls time flow for rest of graph. */
class sink_node : public node {
protected:
	virtual void setup() { }
	virtual void pre_process(time_unit t) { }
	virtual void process(node_job&) = 0;
	
public:	
	template<std::size_t Dim, typename Elem>
	using input_type = node_input_wrapper<node_input, Dim, Elem>;

	template<typename Value>
	using parameter_type = node_parameter<Value>;

	explicit sink_node(graph& gr) : node(gr) { }
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
	bool process_next_frame() final override;
	
	void setup_graph();
	
	void pull(time_unit t);
	void pull_next_frame() { process_next_frame(); }
	
	void seek(time_unit t);
};


}}

#endif
