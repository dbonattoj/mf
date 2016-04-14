#ifndef MF_FLOW_NODE_BASE_H_
#define MF_FLOW_NODE_BASE_H_

#include <vector>
#include <memory>
#include <atomic>
#include <string>
#include <ostream>
#include "../common.h"
#include "node_io_base.h"

namespace mf { namespace flow {

/// Base class for node in flow graph.
class node_base {
protected:
	std::vector<node_input_base*> inputs_; ///< Inputs of this node.
	std::vector<node_output_base*> outputs_; ///< Outputs of this node.
	
	bool was_setup_ = false;

	time_unit prefetch_duration_ = -1;
	time_unit stream_duration_ = -1;
	bool seekable_ = false;
	time_unit offset_ = -1;

	std::atomic<time_unit> time_{-1}; ///< Time of last processed frame by this node.


	void define_source_stream_properties(bool seekable, time_unit stream_duration = -1);


			
	/// Define offset of this node, and of preceding nodes.
	/** Recursively also sets offsets of preceding nodes. If offset was already set, it is updated and propagated
	 ** only when new value is larger. */
	void propagate_offset_(time_unit offset);
	
	/// Define required durations of output buffers.
	/** Sets output buffer durations of preceding nodes (not of this node's outputs). Then recurses into preceding
	 ** nodes. */
	void propagate_output_buffer_durations_();
		
	/// Set up preceding nodes, and then this node.
	/** Calls setup_() on concrete subclass, calls setup_() on outputs, and sets did_setup_. Does not re-initialize
	 ** when did_setup_ is already set. */
	void propagate_setup_();
	
	
	/// Set up the node.
	/** Implemented in concrete subclass. Must define frame shapes of outputs. Preceding nodes are already set up
	 ** when this is called, allowing node to define output frame shapes in function of input frame shapes. */
	virtual void setup() { }
	
		
	/// Process current frame.
	/** Implemented in concrete subclass. Input and output views are made available while in this function. Subclass
	 ** must read frame(s) from input(s), process, and write one frame into output(s). */
	virtual void process() = 0;
	
		
private:
	friend node_input_base::node_input_base(node_base&, time_unit, time_unit);
	friend node_output_base::node_output_base(node_base&);

	void register_input_(node_input_base&);	
	void register_output_(node_output_base&);

	void deduce_stream_properties_();
	
public:
	std::string name;
	
	bool was_setup() const { return was_setup_; }
	bool was_launched() const { return was_launched_; }
	
	virtual void launch() { }
	virtual void stop() { }
	
	bool is_seekable() const { return seekable_; }
	time_unit stream_duration() const { return stream_duration_; }
		
	/// Time of last processed frame.
	/** When currently processing a frame, time of that frame. */
	time_unit current_time() const noexcept { return time_; }
	
	/// Absolute offset, relative to graph sink node.
	/** Maximal number of frames that this node can be in advance relative to graph sink node. -1 when not yet defined.
	 ** Gets computed in propagate_offset_(), during graph setup. Used to determine output buffer durations in graph. */
	time_unit offset() const noexcept { return offset_; }
		
	bool is_active() const;
	
	bool is_source() const { return (inputs_.size() == 0); }
	bool is_sink() const { return (outputs_.size() == 0); }

	#ifndef NDEBUG
	void debug_print(std::ostream&) const;
	#endif
};

}}

#endif
