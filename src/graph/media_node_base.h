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
	
	bool did_setup_ = false; ///< Set to true after node was set up.
	time_unit offset_ = -1; ///< Absolute offset, relative to graph sink node.
	std::atomic<time_unit> time_{-1}; ///< Current time, i.e. time of last processed frame.
	std::atomic<bool> reached_end_{false}; ///< True when last processed frame is last in stream.
			
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
	
	void propagate_stop_();
	virtual void stop_() = 0;

	
	/// Set up the node.
	/** Implemented in concrete subclass. Must set frame shapes of outputs. Preceding nodes are already set up
	 ** when this is called. */
	virtual void setup_() { }
	
	/// Process current frame.
	/** Implemented in concrete subclass. Input and output views are made available while in this function. Subclass
	 ** must read frame(s) from input(s), process, and write one frame into output(s). */
	virtual void process_() = 0;
	
	/// Check whether node reached its end.
	/** Must return true when the last processed frame is the last frame. process_() will not be called after this
	 ** returned true. */
	virtual bool process_reached_end_() { return false; }
	
	explicit media_node_base(time_unit prefetch) :
		prefetch_duration_(prefetch) { }

private:
	friend media_node_input_base::media_node_input_base(media_node_base&, time_unit, time_unit);
	friend media_node_output_base::media_node_output_base(media_node_base&);

	void register_input_(media_node_input_base&);	
	void register_output_(media_node_output_base&);

	
public:
	std::string name;

	virtual ~media_node_base() { }
		
	/// Time last processed frame.
	/** When currently processing a frame, time of that frame. */
	time_unit current_time() const noexcept { return time_; }
	
	/// Returns true when no more frame can be pulled.
	/** Either because process_reached_end_() returns true, or because an input has reached end. */
	bool reached_end() const { return reached_end_; }

	/// Absolute offset, relative to graph sink node.
	/** Maximal number of frames that this node can be in advance relative to graph sink node. -1 when not yet defined.
	 ** Gets computed in propagate_offset_(), during graph setup. Used to determine output buffer durations in graph. */
	time_unit offset() const noexcept { return offset_; }

	/// Pull frames until \a target_time.
	/** Pulls until \a target_time, or until end, whichever comes first. Implemented by subclass (media_sequential_node
	 ** or media_parallel_node). Must ensure that after call, frame \a target_time will eventually be available in
	 ** output buffer(s), and/or end time will be set in buffer(s). Cannot be called when reached_end() is true. */
	virtual void pull(time_unit target_time) = 0;
	
	void print(std::ostream& str) const;
};

}

#endif
