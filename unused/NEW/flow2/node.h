#ifndef MF_FLOW_NODE_H_
#define MF_FLOW_NODE_H_

#include "../common.h"
#include "../queue/frame.h"
#include <atomic>
#include <vector>
#include <functional>

namespace mf { namespace flow {

class node_output;
class node_input;

/// Node in flow graph, base class.
class node {
private:
	std::vector<std::reference_wrapper<node_output>> outputs_;
	std::vector<std::reference_wrapper<node_input>> inputs_;

	bool was_setup_ = false;
	time_unit prefetch_duration_ = 0;
	time_unit offset_ = -1;
	time_unit stream_duration_ = -1;
	bool seekable_ = false;
	bool bounded_ = false;
	
	std::atomic<time_unit> current_time_ = -1; ///<  Time of last/current frame processed by node.
	time_unit pull_time_ = -1; ///< Time of last frame pulled on node's output(s).
	
	void propagate_offset_(time_unit offset);
	void propagate_required_output_buffer_durations_();
	void propagate_setup_();
	void deduce_stream_properties_();

	void propagate_activation_();

protected:
	class job;
	
	node() = default;
	node(const node&) = delete;
	node& operator=(const node&) = delete;
	
	const auto& inputs() const { return inputs_; }
	const auto& outputs() const { return outputs_; }
	
	void define_source_stream_properties(bool seekable, time_unit stream_duration = -1);
	void set_prefetch_duration(time_unit);
	
	void setup_sink();

	void set_pull_time(time_unit t) noexcept { pull_time_ = t; }
	void set_current_time(time_unit t) noexcept { current_time_ = t; }

	virtual void setup() { }

public:
	virtual ~node() { }
	
	virtual void launch() = 0;
	virtual void stop() = 0;
	virtual void pull(time_unit t) = 0;

	bool was_setup() const noexcept { return was_setup_; }

	bool is_source() const noexcept { return inputs_.empty(); }
	bool is_sink() const noexcept { return outputs_.empty(); }

	time_unit prefetch_duration() const noexcept { return prefetch_duration_; }

	bool stream_duration_is_defined() const noexcept { return (stream_duration_ != -1); }
	time_unit stream_duration() const noexcept { return stream_duration_; }
	bool is_seekable() const noexcept { return seekable_; }
	bool is_bounded() const;

	bool is_active() const noexcept { MF_EXPECTS(was_setup_); return active_; }

	time_unit current_time() const noexcept { return current_time_; }
	time_unit pull_time() const noexcept { return pull_time_; }	
};


/// Work unit of flow graph node.
/** Contains input and output views for concrete node to read/write to. */
class node::job {
private:
	node& node_;
	time_unit time_;
	bool end_marked_ = false;
	std::vector<timed_frames_view> input_views_;
	std::vector<frame_view> output_views_;

public:
	job(node&, time_unit t);
	~job();
	
	void open(const node_input&);
	void open(const node_output&);
	void close_all();
	
	time_unit time() const noexcept { return time_; }
	
	void mark_end();

	template<typename Input>
	auto in_full(Input& port) {
		return from_generic_timed<Input::dimension, typename Input::elem_type>(
			input_views_.at(port.index()),
			port.frame_shape()
		);
	}

	template<typename Input>
	auto in(Input& port) {
		return in_full(port).at_time(time_);
	}	

	template<typename Output>
	auto out(Output& port) {
		return from_generic_frame<Input::dimension, typename Input::elem_type>(
			output_views_.at(port.index()),
			port.frame_shape()
		);
	}
};


/// Output port of node in flow graph.
class node_output {
private:
	node& node_;
	std::ptrdiff_t index_;
	
	frame_properties frame_properties_;
	time_unit required_buffer_duration_ = -1;
	
	bool active_ = true;

protected:
	node_output(node& nd, std::ptrdiff_t i) :
		node_(nd), index_(i) { }
	node_output(const node_output&) = delete;

public:
	std::ptrdiff_t index() const noexcept { return index_; }
	node& this_node() const noexcept { return node_; }
	
	void define_frame_properties(const frame_properties&);
	const frame_properties& get_frame_properties() const;
	
	void define_required_buffer_duration(time_unit dur);
	time_unit get_required_buffer_duration() const;
	
	virtual bool can_setup() const noexcept = 0;
	virtual void setup() = 0;
	
	bool is_active() const noexcept { return active_; }
	void propagate_activation(bool connected_input_activated);
	
	/// \name Read interface, used by connected input.
	/// Constitutes access point to the node from other nodes in graph.
	///@{
	virtual void pull(time_unit t) = 0;
	virtual timed_frames_view begin_read(time_unit duration) = 0;
	virtual void end_read(time_unit duration) = 0;
	virtual time_unit end_time() const = 0;
	///@}
	
	/// \name Write interface, used by node.
	/// Implemented differently for different node types. 
	///@{
	virtual frame_view begin_write_frame(time_unit& t) = 0;
	virtual void end_write_frame(bool was_last_frame) = 0;
	///@}
};


/// Input port of node in flow graph.
class node_input {
private:
	node& node_;
	std::ptrdiff_t index_;

	node_output* connected_output_ = nullptr;

	time_unit past_window_ = 0;
	time_unit future_window_ = 0;
	
protected:
	node_input(node& nd, std::ptrdiff_t i) :
		node_(nd), index_(i) { }
	node_input(const node_input&) = delete;

public:
	std::ptrdiff_t index() const noexcept { return index_; }
	node& this_node() const noexcept { return node_; }
	
	time_unit past_window_duration() const noexcept { return past_window_; }
	time_unit future_window_duration() const noexcept { return future_window_; }
	
	void connect(node_output&);
	bool is_connected() const noexcept { return (connected_output_ != nullptr); }
	node_output& connected_output() const noexcept { return *connected_output_; }
	node& connected_node() const noexcept { return connected_output().this_node(); }

	bool is_active() const noexcept;

	/// \name Read interface, used by node.
	///@{
	void pull(time_unit t);
	timed_frames_view begin_read_frame(time_unit t);
	void end_read_frame(time_unit t);
	///@}
};


}}

#endif
