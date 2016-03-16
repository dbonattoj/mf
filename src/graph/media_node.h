#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include <vector>
#include <memory>
#include "../common.h"
#include "media_node_io_base.h"

namespace mf {

/// Base class for nodes of media nodes.
class media_node {
protected:
	std::vector<media_node_input_base*> inputs_; ///< Inputs of this node.
	std::vector<media_node_output_base*> outputs_; ///< Outputs of this node.
	
	bool did_setup_ = false; ///< Set to true after node was set up.
	time_unit prefetch_duration_ = 0; ///< Maximal number of frames node may process in advance.
	time_unit offset_ = -1; ///< Absolute offset, relative to graph sink node.
	time_unit time_ = -1; ///< Current time, i.e. time of last processed frame.
	bool reached_end_ = false; ///< True when last processed frame is last in stream.
			
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
	/** Implemented in concrete subclass. Must set frame shapes of outputs. Preceding nodes are already set up
	 ** when this is called. */
	virtual void setup_() = 0;
	
	/// Process current frame.
	/** Implemented in concrete subclass. Input and output views are made available while in this function. */
	virtual void process_() = 0;
	
	/// Returns true when last processed frame is last frame.
	/** Optionally implemented in concrete subclass. Default implementation returns false. If this returns true, or
	 ** if any input reached end, then reached_end() on this node becomes true. */
	virtual bool process_reached_end_() const;
	
	explicit media_node(time_unit prefetch) :
		prefetch_duration_(prefetch) { }

private:
	friend media_node_input_base::media_node_input_base(media_node&, time_unit, time_unit);
	friend media_node_output_base::media_node_output_base(media_node&);

	void register_input_(media_node_input_base&);	
	void register_output_(media_node_output_base&);

	
public:
	virtual ~media_node() { }
		
	/// Time last processed frame.
	/** When currently processing a frame, time of that frame. */
	time_unit current_time() const noexcept { return time_; }
	
	/// Returns true when last processed frame is last in stream.
	/** Either because process_reached_end_() returned true for last frame, or because an input has reached end. */
	bool reached_end() const { return reached_end_; }

	/// Pull frames until \a target_time.
	/** Pulls until \a target_time, or until end, whichever comes first. Implemented by subclass (media_sequential_node
	 ** or media_parallel_node). Must ensure that after call, frame \a target_time will eventually be available in
	 ** output buffer(s), and/or end time will be set in buffer(s). */
	virtual void pull_frames(time_unit target_time) = 0;
};

}

#endif
