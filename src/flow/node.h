/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
	time_unit min_offset_ = -1; ///< Minimal number of frames that node can be in advance of sink.
	time_unit max_offset_ = -1; ///< Maximal number of frames that node can be in advance of sink.
	time_unit stream_duration_ = -1; ///< Total duration of stream for this node, or -1 if undetermined.
	bool seekable_ = false; ///< Whether this node can handle seek request from input.
	
	std::atomic<time_unit> current_time_ {-1}; ///<  Time of last/current frame processed by node.
	std::atomic<time_unit> end_time_ {-1}; ///< End time of stream, or -1. Always defined when end reached.
	
	void propagate_offset_(time_unit min_off, time_unit max_off); ///< Define this node's offsets and preceding node's.
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
	[[deprecated]] bool reached_end() const noexcept;

	bool was_setup() const noexcept { return was_setup_; }

	time_unit prefetch_duration() const noexcept { return prefetch_duration_; }
	time_unit min_offset() const noexcept { return min_offset_; }
	time_unit max_offset() const noexcept { return max_offset_; }
	bool stream_duration_is_defined() const noexcept { return (stream_duration_ != -1); }
	time_unit stream_duration() const noexcept { return stream_duration_; }
	bool is_seekable() const noexcept { return seekable_; }
	bool is_bounded() const;

	[[deprecated]] time_unit current_time() const noexcept { return current_time_; }
};

/// Output port of another node, read interface for connected node.
/** Constitutes access point to the node from other nodes in graph. Polymorphic abstract class with interface
 ** functions. Implemented by `node_output` itself for a direct connection where generic frames are passed through edge.
 ** Filters implement alternate versions. */
class node_remote_output {
public:
	virtual node_output& this_output() noexcept = 0;

	virtual bool may_pull() { return true; }
	
	/// Pull the frames for time span \a span from the node.
	/** Must be called prior to begin_read(). Ensures that `span.duration()` frames can be read using begin_read()
	 ** afterwards, and that the timed span returned by begin_read() will start at `span.start_time()`. If near
	 ** and, only pulls to end, and span returned by begin_read() will be truncated.
	 ** For synchronous node types, this recursively pulls and processes frames, while begin_read() just reads them. */
	virtual void pull(time_span span) = 0;
	
	/// Begin reading `duration` frames pulled previously.
	/** Returns timed frame array view `vw` with `vw.duration() == duration` and `vw.start_time() == span.start_time()`
	 ** where `span` is the time span previously passed to `pull()`. Near end of stream, the returned view may be
	 ** truncated. May return null view in an error condition.
	 ** The view remains available for reading until the call to end_read(). pull() or begin_read() must not be called
	 ** again before call to end_read(). */
	virtual timed_frame_array_view begin_read(time_unit duration) = 0;
	
	/// End reading, and mark \a duration frames as having been read.
	/** Must be called after begin_read() call. \a duration must be less or equal to `duration` argument of preceding
	 ** begin_read() call. This informs the node that `duration` frames were read, and next pull/read will likely
	 ** occur for time `vw.start_time() + duration`. The view `vw` that was returned by begin_read() cannot be accessed
	 ** after the end_read() call. */
	virtual void end_read(time_unit duration) = 0;
	
	/// Stream end time of node, or -1 if currently undefined.
	/** If pull() or begin_read() call crossed end (and the span/duration got truncated), then end_time() must return
	 ** the stream end time (and not -1) after the call to pull() or begin_read(). */
	virtual time_unit end_time() const = 0;
};


/// Output port of node in flow graph.
class node_output : public node_remote_output {
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
	node_output& this_output() noexcept final override { return *this; }
	
	void define_frame_length(std::size_t len) { frame_length_ = len; }
	std::size_t frame_length() const noexcept { return frame_length_; }

	void define_format(const frame_format& format) { format_ = format; }
	const frame_format& format() const noexcept { return format_; }

	bool is_connected() const noexcept { return (connected_input_ != nullptr); }
	node_input& connected_input() const noexcept { MF_EXPECTS(is_connected()); return *connected_input_; }
	void input_has_connected(node_input&);
	
	// TODO adjust format for thin node series
	virtual void setup() = 0;
			
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
	
	node_remote_output* connected_output_ = nullptr;
		
	bool activated_ = true;
	
protected:
	node_input(const node_input&) = delete;

public:
	node_input(node& nd, std::ptrdiff_t index, time_unit past_window, time_unit future_window);

	std::ptrdiff_t index() const noexcept { return index_; }
	node& this_node() const noexcept { return node_; }

	time_unit past_window_duration() const noexcept { return past_window_; }
	time_unit future_window_duration() const noexcept { return future_window_; }
	
	void connect(node_remote_output&);
	bool is_connected() const noexcept { return (connected_output_ != nullptr); }
	node_remote_output& connected_output() const noexcept { MF_EXPECTS(is_connected()); return *connected_output_; }
	node& connected_node() const noexcept { MF_EXPECTS(is_connected()); return connected_output().this_output().this_node(); }

	bool is_activated() const noexcept { return activated_; }
	void set_activated(bool);

	/// \name Read interface, used by node.
	///@{
	void pull(time_unit t);
	timed_frame_array_view begin_read_frame(time_unit t);
	void end_read_frame(time_unit t);
	void cancel_read_frame();
	time_unit end_time() const { return connected_output_->end_time(); }
	///@}
};


}}

#endif
