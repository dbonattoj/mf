#ifndef MF_FLOW_NODE_IO_BASE_H_
#define MF_FLOW_NODE_IO_BASE_H_

#include <ostream>
#include "../common.h"

namespace mf { namespace flow {

class node_base;

/// Abstract base class for media node output.
class node_output_base {
protected:
	/// Media node that this output belongs to.
	node_base& node_;
	
	time_unit required_buffer_duration_ = -1;
	
	bool input_activated_ = true;
		
public:
	explicit node_output_base(node_base&);

	node_base& node() const { return node_; }

	virtual void setup() = 0;

	virtual time_unit begin_write() = 0;
	virtual void end_write(bool is_last_frame) = 0;

	virtual void didnt_write() = 0;

	virtual void skip(time_unit t) = 0;


	void define_required_buffer_duration(time_unit dur) { required_buffer_duration_ = dur; }
	time_unit required_buffer_duration() const { return required_buffer_duration_; }
	bool required_buffer_duration_is_defined() const { return (required_buffer_duration_ != -1); }	
		
	virtual bool frame_shape_is_defined() const = 0;

	/// Called by connected input when it is activated or desactivated.
	/**  */
	void propagate_activation(bool input_activated);
	
	bool input_is_activated() const { return input_activated_; }

	bool is_active() const;
	
	#ifndef NDEBUG
	virtual void debug_print(std::ostream&) const = 0;
	#endif
};



/// Abstract base class for media node input.
class node_input_base {
protected:
	time_unit past_window_ = 0;
	time_unit future_window_ = 0;
	
	/// Whether input is currently activated.
	/** Controlled by node using activate() and desactivate().
	 ** If false, node will not receive frames from this input. Determines if preceding nodes in graph are active. */
	bool activated_ = true;

public:	
	node_input_base(node_base&, time_unit past_window, time_unit future_window);

	time_unit past_window_duration() const noexcept { return past_window_; }
	time_unit future_window_duration() const noexcept { return future_window_; }

	virtual void begin_read(time_unit t) = 0;
	virtual void end_read(time_unit t) = 0;
	virtual void skip(time_unit t) = 0;
	virtual bool reached_end() const = 0;
	
	virtual node_output_base& connected_output() const = 0;
	node_base& connected_node() const { return this->connected_output().node(); }
	
	bool is_activated() const { return activated_; }
	
	void activate();
	void desactivate();
	void set_activated(bool a) { if(a) activate(); else desactivate(); }
};


}}

#endif
