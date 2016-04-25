#ifndef MF_FLOW_NODE_BASE_H_
#define MF_FLOW_NODE_BASE_H_

#include <vector>
#include <atomic>
#include <functional>
#include "../ndarray/ndarray_timed_view.h"
#include "../common.h"

namespace mf { namespace flow {

/// Base class for node in flow graph.
class node_base {
public:
	class input_base;
	class output_base;
	template<std::size_t Dim, typename Elem> class input;
	template<std::size_t Dim, typename Elem> class output;

private:
	std::vector<std::reference_wrapper<input_base>> inputs_;
	std::vector<std::reference_wrapper<output_base>> outputs_;
	
	bool was_setup_ = false;
	time_unit prefetch_duration_ = 0;
	time_unit stream_duration_ = -1;
	bool seekable_ = false;
	time_unit offset_ = -1;
	
	bool active_ = true;
	std::atomic<time_unit> time_{-1};
	
	void propagate_offset_(time_unit offset);
	void propagate_output_buffer_durations_();
	void propagate_setup_();
	void deduce_stream_properties_();

	void propagate_activation_();

protected:
	const auto& inputs() const { return inputs_; }
	const auto& outputs() const { return outputs_; }

	void define_source_stream_properties(bool seekable, time_unit stream_duration = -1);
	void set_prefetch_duration(time_unit);
	
	void setup_sink();

	void set_current_time(time_unit t) noexcept { time_ = t; }

	/// \name Concrete node functions.
	/// Implemented by concrete subclass which implements the processing done by the node.
	///@{
	virtual void setup() { }
	virtual void pre_process() { }
	virtual void process() = 0;
	virtual bool reached_end() const noexcept { return false; }
	///@}

	node_base() = default;
	node_base(const node_base&) = delete;
	node_base& operator=(const node_base&) = delete;
	
public:
	std::string name;

	virtual ~node_base() { }
	
	bool was_setup() const noexcept { return was_setup_; }
	time_unit prefetch_duration() const noexcept { return prefetch_duration_; }
	bool stream_duration_is_defined() const noexcept { return (stream_duration_ != -1); }
	time_unit stream_duration() const noexcept { return stream_duration_; }
	bool is_seekable() const noexcept { return seekable_; }

	time_unit offset() const noexcept { MF_EXPECTS(was_setup_); return offset_; }

	bool is_bounded() const;
	bool is_source() const noexcept { return inputs_.empty(); }
	bool is_sink() const noexcept { return outputs_.empty(); }
	
	bool is_active() const noexcept { MF_EXPECTS(was_setup_); return active_; }
	time_unit current_time() const noexcept { MF_EXPECTS(was_setup_); return time_; }
		
	virtual void launch() { }
	virtual void stop() { }
};


/// Base class for node output.
class node_base::output_base {
private:
	node_base& node_;
	time_unit required_buffer_duration_ = -1;

	bool active_ = true;
	

	output_base(const output_base&) = delete;

protected:
	explicit output_base(node_base&);

public:
	node_base& node() const noexcept { return node_; }

	void define_required_buffer_duration(time_unit dur) { required_buffer_duration_ = dur; }
	time_unit required_buffer_duration() const noexcept { return required_buffer_duration_; }
	bool required_buffer_duration_is_defined() const noexcept { return (required_buffer_duration_ != -1); }

	virtual bool can_setup() const noexcept = 0;
	virtual void setup() = 0;
	
	void propagate_activation(bool input_activated);

	bool is_active() const noexcept { return active_; }

	/// \name Write interface.
	///@{
	virtual bool begin_write_next_frame(time_unit&) = 0;
	virtual void end_write_frame(bool mark_end = false) = 0;
	virtual void cancel_write_frame() = 0;
	///@}
};


template<std::size_t Dim, typename Elem>
class node_base::output : public output_base {
public:
	using frame_view_type = ndarray_view<Dim, Elem>;
	using full_view_type = ndarray_timed_view<Dim + 1, Elem>;
	using frame_shape_type = typename frame_view_type::shape_type;

private:
	frame_shape_type frame_shape_;

	bool view_available_ = false;
	frame_view_type view_;

protected:
	void set_view(const frame_view_type& view);
	void unset_view();
	
public:
	explicit output(node_base& nd) : output_base(nd) { }

	void define_frame_shape(const frame_shape_type& shp) { frame_shape_ = shp; }
	const frame_shape_type& frame_shape() const noexcept { return frame_shape_; }
	bool frame_shape_is_defined() const noexcept { return frame_shape_.size() != 0; }

	bool can_setup() const noexcept override;

	bool view_is_available() const noexcept { return view_available_; }
	const frame_view_type& view() { MF_EXPECTS(view_available_); return view_; }
	
	/// \name Read interface.
	/// Used by connect input \ref node_base::input.
	///@{
	/// Begin reading time span \a span from the output.
	/** Depending on connected node type, may process frame(s) (sync) or wait for them to become available (async).
	 ** Connected node seeks if necessary. */
	virtual bool begin_read_span(time_span span, full_view_type& view) = 0;

	/// End reading from output.
	/** If for sequential read, the next call to begin_read_span() will not include the first frame from the time span,
	 ** of the last call, then \a consume_frame must be set. */
	virtual void end_read(bool consume_frame) = 0;

	/// End time of output, or -1 if not known.
	/** If the last frame of the view returned by the previous call to begin_read_span() was the last frame of the
	 ** output stream, then end_time() must return that time. Otherwise, may return -1 when the end time is in the
	 ** future, and the node is seekable. */
	virtual time_unit end_time() const = 0;
	
	
	///@}
};


/// Base class for node input.
class node_base::input_base {
private:
	node_base& node_;
	time_unit past_window_ = 0;
	time_unit future_window_ = 0;
	bool activated_ = true;

	input_base(const input_base&) = delete;

protected:
	input_base(node_base& nd, time_unit past_window, time_unit future_window);

public:
	node_base& node() const noexcept { return node_; }

	virtual output_base& connected_output() const = 0;
	virtual node_base& connected_node() const = 0;

	time_unit past_window_duration() const noexcept { return past_window_; }
	time_unit future_window_duration() const noexcept { return future_window_; }

	bool is_activated() const noexcept { return activated_; }
	void set_activated(bool);
		
	/// \name Read interface.
	/// Used by node to read from input.
	///@{
	virtual bool begin_read_frame(time_unit) = 0;
	virtual void end_read_frame(time_unit) = 0;
	virtual bool reached_end(time_unit t) const = 0;
	///@}
};


template<std::size_t Dim, typename Elem>
class node_base::input : public input_base {
public:
	using output_type = output<Dim, Elem>;

	using frame_view_type = ndarray_view<Dim, Elem>;
	using full_view_type = ndarray_timed_view<Dim + 1, Elem>;
	using frame_shape_type = typename frame_view_type::shape_type;
	
private:
	output_type* connected_output_ = nullptr;
	
	bool view_available_ = false;
	full_view_type view_;

public:
	input(node_base& nd, time_unit past_window = 0, time_unit future_window = 0) :
		input_base(nd, past_window, future_window) { }

	const frame_shape_type& frame_shape() const noexcept { return connected_output().frame_shape(); }

	void connect(output_type&);
	bool is_connected() const noexcept { return (connected_output_ != nullptr); }
	
	output_type& connected_output() const final override;
	node_base& connected_node() const final override { return connected_output().node(); }

	bool view_is_available() const noexcept { return view_available_; }
	const full_view_type& full_view() { MF_EXPECTS(view_available_); return view_; }
	frame_view_type view();
	
	std::ptrdiff_t full_view_center() const noexcept {
		return (node().current_time() - view_.start_time());
	}
	
	/// \name Read interface.
	///@{
	/// Begin reading at time \arg t.
	/** Frames in time window around \arg t become available through view(). Window may be truncated near beginning
	 ** and end of stream.  \a t must be lower than the end time of the stream. */
	bool begin_read_frame(time_unit t) final override;
	void end_read_frame(time_unit t) final override;
	bool reached_end(time_unit t) const final override;
	///@}
};

}}

#include "node_base.tcc"

#endif
