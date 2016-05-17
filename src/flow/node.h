#ifndef MF_FLOW_NODE_H_
#define MF_FLOW_NODE_H_

#include "../common.h"
#include "../queue/frame.h"
#include <vector>
#include <atomic>
#include <string>
#include <memory>

namespace mf { namespace flow {

class graph;
class node_output;
class node_input;
class node_job;

/// Node in flow graph, base class.
class node {
private:
	graph& graph_; ///< Graph to which this node belongs.
	std::vector<std::unique_ptr<node_output>> outputs_; ///< Outputs, by reference.
	std::vector<std::unique_ptr<node_input>> inputs_; ///< Inputs, by reference.

	bool was_setup_ = false; ///< True after node was set up.
	time_unit prefetch_duration_ = 0; ///< Maximal number of frames after current time that node may prefetch.
	time_unit offset_ = -1; ///< Maximal number of frames that node can be in advance of sink.
	time_unit stream_duration_ = -1; ///< Total duration of stream for this node, or -1 if undetermined.
	bool seekable_ = false; ///< Whether this node can handle seek request from input.
	
	bool active_ = true;
	std::atomic<time_unit> current_time_ {-1}; ///<  Time of last/current frame processed by node.
	std::atomic<time_unit> end_time_ {-1}; ///< End time of stream, or -1. Always defined when end reached.
	
	void propagate_offset_(time_unit offset); ///< Define this node's offset and then recursively preceding node's.
	void propagate_setup_(); ///< Recursively set up outputs and node of preceding nodes, and then this node.
	void deduce_stream_properties_(); ///< Define stream properties of non-source node based on input nodes.

protected:	
	explicit node(graph& gr) : graph_(gr) { }
	node(const node&) = delete;
	node& operator=(const node&) = delete;
		
	void setup_sink(); ///< Called by sink node, runs set up procedure for all node in graph.

	void set_current_time(time_unit t) noexcept { current_time_ = t; }
	void mark_end() { end_time_ = current_time_ + 1; }
	
	node_job make_job();
	void cancel_job(node_job&);
	
	template<typename Input>
	Input& add_input_(time_unit past_window, time_unit future_window) {
		Input* input = new Input(*this, inputs_.size(), past_window, future_window);
		inputs_.emplace_back(input);
		return *input;
	}

	template<typename Output>
	Output& add_output_(const frame_format& format) {
		Output* output = new Output(*this, outputs_.size(), format);
		outputs_.emplace_back(output);
		return *output;
	}
	
public:
	std::string name;

	virtual ~node() { }
	
	graph& this_graph() noexcept { return graph_; }

	void define_source_stream_properties(bool seekable, time_unit stream_duration = -1);
	void set_prefetch_duration(time_unit);	

	const auto& inputs() noexcept { return inputs_; }
	const auto& outputs() noexcept { return outputs_; }

	bool is_source() const noexcept { return inputs_.empty(); }
	bool is_sink() const noexcept { return outputs_.empty(); }
	
	virtual void internal_setup() = 0; ///< Called by propagate_setup_.
	virtual void launch() = 0; ///< Called by graph for all nodes, before any frame is pulled from sink.
	virtual void stop() = 0; ///< Called by graph for all node, before destruction of any node.
	
	virtual bool process_next_frame() = 0;
	
	time_unit end_time() const noexcept { return end_time_; }
	bool reached_end() const noexcept;

	bool was_setup() const noexcept { return was_setup_; }

	time_unit prefetch_duration() const noexcept { return prefetch_duration_; }
	time_unit offset() const noexcept { return offset_; }
	bool stream_duration_is_defined() const noexcept { return (stream_duration_ != -1); }
	time_unit stream_duration() const noexcept { return stream_duration_; }
	bool is_seekable() const noexcept { return seekable_; }
	bool is_bounded() const;

	bool is_active() const noexcept { MF_EXPECTS(was_setup_); return active_; }
	void update_activation(); ///< Called by output, propagates to preceding nodes.

	time_unit current_time() const noexcept { return current_time_; }
};


/// Output port of node in flow graph.
class node_output {
private:
	node& node_;
	std::ptrdiff_t index_ = -1;
	
	node_input* connected_input_ = nullptr;
	frame_format format_;
	std::size_t frame_length_;
	
	bool active_ = true;

protected:
	node_output(const node_output&) = delete;

public:
	node_output(node& nd, std::ptrdiff_t index, const frame_format&);
	
	virtual ~node_output() { }

	std::ptrdiff_t index() const noexcept { return index_; }
	node& this_node() const noexcept { return node_; }
	
	void define_frame_length(std::size_t len) { frame_length_ = len; }
	std::size_t frame_length() const noexcept { return frame_length_; }

	void define_format(const frame_format& format) { format_ = format; }
	const frame_format& format() const noexcept { return format_; }

	bool is_connected() const noexcept { return (connected_input_ != nullptr); }
	node_input& connected_input() const noexcept { MF_EXPECTS(is_connected()); return *connected_input_; }
	void input_has_connected(node_input&);
	
	// TODO adjust format for thin node series
	virtual void setup() = 0;
	
	bool is_active() const noexcept { return active_; }
	void propagate_activation(bool active);
	
	/// \name Read interface, used by connected input.
	/// Constitutes access point to the node from other nodes in graph.
	///@{
	virtual void pull(time_span span) = 0;
	virtual timed_frames_view begin_read(time_unit duration) = 0;
	virtual void end_read(time_unit duration) = 0;
	virtual time_unit end_time() const = 0;
	///@}
	
	/// \name Write interface, used by node.
	/// Implemented differently for different node types. 
	///@{
	virtual frame_view begin_write_frame(time_unit& t) = 0;
	virtual void end_write_frame(bool was_last_frame) = 0;
	virtual void cancel_write_frame() = 0;
	///@}
};


/// Input port of node in flow graph.
class node_input {
private:
	node& node_;
	std::ptrdiff_t index_ = -1;

	time_unit past_window_ = 0;
	time_unit future_window_ = 0;
	node_output* connected_output_ = nullptr;
	
	bool activated_ = true;
	
protected:
	node_input(const node_input&) = delete;

public:
	node_input(node& nd, std::ptrdiff_t index, time_unit past_window, time_unit future_window);

	std::ptrdiff_t index() const noexcept { return index_; }
	node& this_node() const noexcept { return node_; }
	
	std::size_t frame_length() const noexcept { return connected_output().frame_length(); }
	const frame_format& format() const noexcept { return connected_output().format(); }
	
	time_unit past_window_duration() const noexcept { return past_window_; }
	time_unit future_window_duration() const noexcept { return future_window_; }
	
	void connect(node_output&);
	bool is_connected() const noexcept { return (connected_output_ != nullptr); }
	node_output& connected_output() const noexcept { MF_EXPECTS(is_connected()); return *connected_output_; }
	node& connected_node() const noexcept { MF_EXPECTS(is_connected()); return connected_output().this_node(); }

	bool is_activated() const noexcept { return activated_; }
	void set_activated(bool);

	/// \name Read interface, used by node.
	///@{
	void pull(time_unit t);
	timed_frames_view begin_read_frame(time_unit t);
	void end_read_frame(time_unit t);
	void cancel_read_frame();
	time_unit end_time() const { return connected_node().end_time(); }
	///@}
};


}}

#endif
