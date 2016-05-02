#ifndef MF_FLOW_SINK_NODE_H_
#define MF_FLOW_SINK_NODE_H_

#include "node.h"
#include "node_io_wrapper.h"

namespace mf { namespace flow {

class graph;

/// Sink node base class.
/** Has one of multiple inputs and no outputs. There is one sink node per graph. Controls time flow for rest of graph. */
class sink_node : public node {
private:
	bool reached_end_ = false;
	
protected:
	virtual void setup() { }
	virtual void pre_process(time_unit t) { }
	virtual void process(job&) = 0;
	
public:	
	explicit sink_node(graph& gr) : node(gr) { }

	template<std::size_t Dim, typename Elem>
	using input_type = node_input_wrapper<node_input, Dim, Elem>;

	explicit sink_node(graph& gr) : node(gr) { }
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
	void pull(time_unit t) final override;
	
	void setup_graph();
	
	void pull_next_frame();
	void seek(time_unit);
	
	bool reached_end() const noexcept { return reached_end_; }
};


}}

#endif
