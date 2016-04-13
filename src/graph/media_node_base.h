#ifndef MF_MEDIA_NODE_BASE_H_
#define MF_MEDIA_NODE_BASE_H_

#include <vector>
#include <memory>
#include <atomic>
#include <string>
#include <ostream>
#include "../common.h"
#include "media_node_io_base.h"

namespace mf {

/// Base class for nodes of media graph.
class media_node_base {
protected:
	std::vector<media_node_input_base*> inputs_; ///< Inputs of this node.
	std::vector<media_node_output_base*> outputs_; ///< Outputs of this node.
	
	bool did_setup_ = false;

	time_unit prefetch_duration_;

	time_unit offset_ = -1;
	time_unit stream_duration_ = -1;

	std::atomic<time_unit> time_{-1}; ///< Current time, i.e. time of last processed frame.
			
	explicit media_node_base(time_unit prefetch_dur) :
		prefetch_duration_(prefetch_dur) { }
	
	/// Define offset of this node, and of preceding nodes.
	/** Recursively also sets offsets of preceding nodes. If offset was already set, it is updated and propagated
	 ** only when new value is larger. */
	void propagate_offset_(time_unit offset);
	
	/// Define required durations of output buffers.
	/** Sets output buffer durations of preceding nodes (not of this node's outputs). Then recurses into preceding
	 ** nodes. */
	void propagate_output_buffer_durations_();
	
	/// Define stream duration of this node and outputs, and of preceding nodes.
	/** Stream duration of this node is set to minimum of stream durations of input nodes. If one is -1, this node's
	 ** also becomes -1. Does nothing if node has no inputs (source nodes define stream duration themselves). 
	 ** Also sets this stream duration of outputs. */
	void propagate_stream_durations_();
	
	/// Set up preceding nodes, and then this node.
	/** Calls setup_() on concrete subclass, calls setup_() on outputs, and sets did_setup_. Does not re-initialize
	 ** when did_setup_ is already set. */
	void propagate_setup_();
	
	void propagate_stop_();
	virtual void stop_() { }

	
	/// Set up the node.
	/** Implemented in concrete subclass. Must define frame shapes of outputs. Preceding nodes are already set up
	 ** when this is called, allowing node to define output frame shapes in function of input frame shapes. */
	virtual void setup_() { }
	
	virtual void launch_() { }
		
	/// Process current frame.
	/** Implemented in concrete subclass. Input and output views are made available while in this function. Subclass
	 ** must read frame(s) from input(s), process, and write one frame into output(s). */
	virtual void process_() = 0;
	
	void define_stream_duration(time_unit dur) { stream_duration_ = dur; }
	time_unit stream_duration() const { return stream_duration_; }
	bool stream_duration_is_defined() const { return (stream_duration_ != -1); }
		
private:
	friend media_node_input_base::media_node_input_base(media_node_base&, time_unit, time_unit);
	friend media_node_output_base::media_node_output_base(media_node_base&);

	void register_input_(media_node_input_base&);	
	void register_output_(media_node_output_base&);

	
public:
	std::string name;

	virtual ~media_node_base() { }
		
	/// Time of last processed frame.
	/** When currently processing a frame, time of that frame. */
	time_unit current_time() const noexcept { return time_; }
	
	/// Absolute offset, relative to graph sink node.
	/** Maximal number of frames that this node can be in advance relative to graph sink node. -1 when not yet defined.
	 ** Gets computed in propagate_offset_(), during graph setup. Used to determine output buffer durations in graph. */
	time_unit offset() const noexcept { return offset_; }
	
	void print(std::ostream& str) const;
	
	bool is_active() const;
};

}

#endif
