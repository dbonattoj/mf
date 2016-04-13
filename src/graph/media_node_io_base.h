#ifndef MF_MEDIA_NODE_IO_BASE_H_
#define MF_MEDIA_NODE_IO_BASE_H_

#include <ostream>
#include "../common.h"

namespace mf {

class media_node_base;

/// Abstract base class for media node output.
class media_node_output_base {
protected:
	/// Media node that this output belongs to.
	media_node_base& node_;
		
	time_unit required_buffer_duration_ = -1;
	time_unit stream_duration_ = -1;
	
	bool active_ = true;
	
public:
	explicit media_node_output_base(media_node_base&);

	media_node_base& node() const { return node_; }

	virtual void setup() = 0;

	virtual time_unit begin_write() = 0;
	virtual void end_write(bool is_last_frame) = 0;

	void define_required_buffer_duration(time_unit dur) { required_buffer_duration_ = dur; }
	time_unit required_buffer_duration() const { return required_buffer_duration_; }
	bool required_buffer_duration_is_defined() const { return (required_buffer_duration_ != -1); }

	void define_stream_duration(time_unit dur) { stream_duration_ = dur; }
	time_unit stream_duration() const { return stream_duration_; }
	bool stream_duration_is_defined() const { return (stream_duration_ != -1); }
		
	virtual bool frame_shape_is_defined() const = 0;
	
	bool is_active() const { return active_; }

	#ifndef NDEBUG
	virtual void debug_print(std::ostream&) const = 0;
	#endif
};



/// Abstract base class for media node input.
class media_node_input_base {
protected:
	time_unit past_window_ = 0;
	time_unit future_window_ = 0;

public:	
	media_node_input_base(media_node_base&, time_unit past_window, time_unit future_window);

	time_unit past_window_duration() const noexcept { return past_window_; }
	time_unit future_window_duration() const noexcept { return future_window_; }

	virtual void begin_read(time_unit t) = 0;
	virtual void end_read(time_unit t) = 0;
	virtual bool reached_end() const = 0;
	
	virtual media_node_output_base& connected_output() const = 0;
};


}

#endif
