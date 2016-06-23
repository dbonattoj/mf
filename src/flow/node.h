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
public:
	enum online_state { online, offline, reconnecting };
	enum pull_result { success, temporary_failure, stopped };

private:
	graph& graph_;
	std::vector<std::unique_ptr<node_output>> outputs_;
	std::vector<std::unique_ptr<node_input>> inputs_;

	std::atomic<online_state> state_ {online};

	bool was_pre_setup_ = false;
	bool was_setup_ = false;
	
	time_unit stream_duration_ = -1;
	bool seekable_ = false;
	
	std::atomic<time_unit> current_time_ {-1};
	
	std::atomic<time_unit> end_time_ {-1}; // TODO for non-seekable (stream_duration_==-1), end_time can be reset when relaunching (new stream)
	
	void propagate_pre_setup_();
	void propagate_setup_();
	void deduce_stream_properties_();

public:
	virtual time_unit minimal_offset_to(const node&) const = 0;
	virtual time_unit maximal_offset_to(const node&) const = 0;
		
	bool precedes(const node&) const;
	bool precedes_strict(const node&) const;
	const node& first_successor() const;

	online_state state() const { return state_; }
	
	void propagate_offline_state();
	void propagate_reconnecting_state();
	void set_online();

public:	
	explicit node(graph& gr) : graph_(gr) { }
	node(const node&) = delete;
	node& operator=(const node&) = delete;
		
	void setup_sink();
	
	void set_current_time(time_unit t) noexcept { current_time_ = t; }
	void mark_end() { end_time_.store(current_time_ + 1); }
	
	node_job make_job();
		
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

	const auto& inputs() const noexcept { return inputs_; }
	const auto& outputs() const noexcept { return outputs_; }

	bool is_source() const noexcept { return inputs_.empty(); }
	bool is_sink() const noexcept { return outputs_.empty(); }
	
	virtual void pre_setup() { }
	virtual void setup() = 0;
	virtual void launch() = 0;
	
	virtual void pre_stop() { }
	virtual void stop() = 0;
		
	time_unit end_time() const noexcept { return end_time_; }
	bool reached_end() const noexcept;

	bool was_setup() const noexcept { return was_setup_; }

	bool stream_duration_is_defined() const noexcept { return (stream_duration_ != -1); }
	time_unit stream_duration() const noexcept { return stream_duration_; }
	bool is_seekable() const noexcept { return seekable_; }
	bool is_bounded() const;

	time_unit current_time() const noexcept { return current_time_; }
};

/// Output port of another node, read interface for connected node.
class node_remote_output {
public:
	virtual node_output& this_output() noexcept = 0;
	
	virtual node::pull_result pull(time_span& span, bool reconnect) = 0;
	
	virtual timed_frame_array_view begin_read(time_unit duration) = 0;
	
	virtual void end_read(time_unit duration) = 0;
	
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
	node& connected_node() const noexcept;
	void input_has_connected(node_input&);
	
	bool is_online() const;

	virtual void setup() = 0;
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
	
public:
	node_input(const node_input&) = delete;

	void set_past_window(time_unit dur) { past_window_ = dur; }
	void set_future_window(time_unit dur) { future_window_ = dur; }
	
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
	node::pull_result pull(time_unit t);
	timed_frame_array_view begin_read_frame(time_unit t);
	void end_read_frame(time_unit t);
	void cancel_read_frame();
	time_unit end_time() const { return connected_output_->end_time(); }
	///@}
};


}}

#endif
