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

#ifndef MF_FLOW_PROCESSING_NODE_H_
#define MF_FLOW_PROCESSING_NODE_H_

#include "node.h"
#include "node_input.h"
#include "node_output.h"
#include <memory>
#include <utility>

namespace mf { namespace flow {

class graph;
class processing_node;
class processing_node_job;


/// Delegate which processes frames of a \ref processing_node, interface.
class processing_node_handler {
public:
	virtual void handler_setup() = 0;
	virtual void handler_pre_process(processing_node_job&) = 0;
	virtual void handler_process(processing_node_job&) = 0;
};

/// Output channel of \ref processing_node_output, base class.
/** \ref processing_node has one \ref processing_node_output with multiple channels.
 ** Derived class contains buffer with the node's output frames, which is created on setup. Frame format and length
 ** must be defined in this class prior to setup.
 ** \ref processing_node_output_channel corresponds to filter output in filter graph. */
class processing_node_output_channel {
private:
	processing_node& node_;
	const std::ptrdiff_t index_;

	frame_format format_;
	std::size_t frame_length_;

protected:
	explicit processing_node_output_channel(processing_node& nd, std::ptrdiff_t index) :
		node_(nd), index_(index) { }
	processing_node_output_channel(const processing_node_output_channel&) = delete;
	processing_node_output_channel& operator=(const processing_node_output_channel&) = delete;
	virtual ~processing_node_output_channel() = default;

public:
	void define_frame_length(std::size_t len) { frame_length_ = len; }
	void define_format(const frame_format& format) { format_ = format; }
	std::size_t frame_length() const noexcept { return frame_length_; }
	const frame_format& format() const noexcept { return format_; }
	
	virtual void setup() = 0;
	
	virtual timed_frame_array_view begin_read(time_unit duration) = 0;
	virtual void end_read(time_unit duration) = 0;
};


/// Output of \ref processing_node.
/** The \ref processing_node has none or one output. Its implementation is delegated to \ref processing_node. */
class processing_node_output : public node_output {
private:
	processing_node& this_node();
	const processing_node& this_node() const;	
	
public:
	using node_output::node_output;
	
	std::size_t channel_count() override const noexcept;
	node::pull_result pull(time_span& span, bool reconnect) override;
	timed_frame_array_view begin_read(time_unit duration, std::ptrdiff_t channel_index) override;
	void end_read(time_unit duration, std::ptrdiff_t channel_index) override;
};


/// Input of \ref processing_node.
/** Adds index value (like \ref processing_node_output_channel) for use with \ref processing_node_job. */
class processing_node_input : public node_input {
private:
	const std::ptrdiff_t index_;

public:
	using node_input::node_input;

	explicit processing_node_input(processing_node& nd, std::ptrdiff_t index) :
		node_input(nd), index_(index) {}
	
	std::ptrdiff_t index() const { return index_; }
};


/// Node which processes frames one-by-one, base class.
/** Frame processing is delegated to external \ref processing_node_handler, for example \ref filter
 ** Has none or one output, which have multiple channels. Can have any number of inputs.
 ** Creates \ref processing_node_job for each frame to process, with indexed input and output views. 
 ** Derived class implements derived class of \ref processing_node_output_channel, which contains buffer for the output
 ** channel. */
class processing_node : public node {
	friend class processing_node_output;

private:
	processing_node_handler* handler_ = nullptr;
	std::vector<std::unique_ptr<processing_node_output_channel>> output_channels_;

protected:
	void verify_connections_validity_() const;

	void handler_setup_();
	void handler_pre_process_(processing_node_job&);
	void handler_process_(processing_node_job&);
	
	processing_node_job begin_job_();
	void finish_job_(processing_node_job&);
	
	timed_frame_array_view output_begin_read_(time_unit duration, std::ptrdiff_t channel_index);
	void output_end_read_(time_unit duration, std::ptrdiff_t channel_index);

	virtual node::pull_result output_pull_(time_span& span, bool reconnect) = 0;
	
public:
	processing_node(graph&, bool with_output);
	~processing_node() override;
	
	void set_handler(processing_node_handler&);
	
	processing_node_input& add_input();
	
	bool has_output() const;
	processing_node_output& output();
	const processing_node_output& output() const;
	std::size_t output_channel_count() const noexcept;	
		
	template<typename Channel, typename... Args>
	Channel& add_output_channel_(Args&&... args) {
		static_assert(std::is_base_of<processing_node_output_channel, Channel>::value,
			"Channel must be derived class of processing_node_output_channel");
		Channel* channel = new Channel(std::forward<Args>(args)...);
		output_channels_.emplace_back(channel);
		return *channel;
	}
};


/// Job of \ref processing_node.
/** Exists during processing of one frame, and is created by the \ref processing_node. Contains readable view
 ** with frames from each activated input, and writable view for frame for each output channel.
 ** Passed to the \ref processing_node_handler. */
class processing_node_job {
private:
	using input_view_handle = std::pair<node_input*, timed_frame_array_view>;
	using output_view_handle = std::pair<processing_node_output_channel*, frame_view>;

	time_unit time_ = -1;
	std::vector<input_view_handle> inputs_;
	std::vector<output_view_handle> output_channels_;

	bool end_marked_ = false;
	
	static bool is_null(const input_view_handle& in) { return (in.first == nullptr); }
	static bool is_null(const output_view_handle& out) { return (out.first == nullptr); }
	static void set_null(input_view_handle& in) { in = { nullptr, timed_frame_array_view::null(); }; }
	static void set_null(input_view_handle& out) { out = { nullptr, frame_view::null(); }; }

public:
	explicit processing_node_job(processing_node& nd);
	~processing_node_job();
	
	bool open_input(processing_node_input&);
	void commit_input(processing_node_input&);
	void cancel_inputs();
	
	bool open_output(processing_node_output_channel&);
	void commit_output(processing_node_output_channel&);
	void cancel_outputs();

	bool end_was_marked() const noexcept { return end_marked_; }
		
	time_unit time() const noexcept { return time_; }
	void mark_end() noexcept { end_marked_ = true; }
	
	bool has_input_view(std::ptrdiff_t index) const noexcept;
	const timed_frame_array_view& input_view(std::ptrdiff_t index);
	
	bool has_output_view(std::ptrdiff_t index) const noexcept;
	const frame_view& output_view(std::ptrdiff_t index);
};



inline processing_node_output& processing_node::output() {
	Expects(outputs().size() >= 1);
	return static_cast<processing_node_output&>(*outputs().front());
}

inline const processing_node_output& processing_node::output() const {
	Expects(outputs().size() >= 1);
	return static_cast<const processing_node_output&>(*outputs().front());
}

inline processing_node& processing_node_output::this_node() {
	return static_cast<processing_node&>(node_output::this_node());
}

inline const processing_node& processing_node_output::this_node() const {
	return static_cast<const processing_node&>(node_output::this_node());
}


}}

#endif
