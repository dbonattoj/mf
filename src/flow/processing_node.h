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

#include "node_derived.h"
#include "node_input.h"
#include "node_output.h"
#include "../nd/ndarray_format.h"
#include <memory>
#include <utility>

namespace mf { namespace flow {

class graph;
class processing_node;
class processing_node_job;


/// Delegate which processes frames of a \ref processing_node, interface.
class processing_node_handler {
public:
	virtual void handler_setup(processing_node&) = 0;
	virtual void handler_pre_process(processing_node&, processing_node_job&) = 0;
	virtual void handler_process(processing_node&, processing_node_job&) = 0;
};


class processing_node_output_channel final {
private:
	processing_node& node_;
	const std::ptrdiff_t index_;
	ndarray_format array_format_;

public:
	processing_node_output_channel(processing_node& nd, std::ptrdiff_t index) :
		node_(nd), index_(index) { }
	
	processing_node& this_node() { return node_; }
	const processing_node& this_node() const { return node_; }
	std::ptrdiff_t index() const { return index_; }
	
	void define_format(const ndarray_format& frm) { array_format_ = frm; }
	const ndarray_format& format() const noexcept { return array_format_; }
};


/// Output of \ref processing_node.
/** The \ref processing_node has no or one output. Its implementation is delegated to \ref processing_node. */
class processing_node_output final : public node_output {
private:
	processing_node& this_node();
	const processing_node& this_node() const;	
	
public:
	using node_output::node_output;
	
	std::size_t channels_count() const noexcept override;
	node::pull_result pull(time_span& span, bool reconnect) override;
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


/// Input of \ref processing_node.
/** Adds index value (like \ref processing_node_output_channel) for use with \ref processing_node_job. */
class processing_node_input final : public node_input {
private:
	const std::ptrdiff_t index_;

public:
	using node_input::node_input;

	processing_node_input(processing_node&, std::ptrdiff_t index);
	
	std::ptrdiff_t index() const { return index_; }
};


/// Node which processes frames one-by-one, base class.
/** Frame processing is delegated to external \ref processing_node_handler, for example \ref filter
 ** Has none or one output, which have multiple channels. Can have any number of inputs.
 ** Creates \ref processing_node_job for each frame to process, with indexed input and output views. 
 ** Derived class implements derived class of \ref processing_node_output_channel, which contains buffer for the output
 ** channel. */
class processing_node : public node_derived<processing_node_input, processing_node_output> {
	using base = node_derived<processing_node_input, processing_node_output>;
	friend class processing_node_output;

public:
	using output_channel_type = processing_node_output_channel;

private:
	processing_node_handler* handler_ = nullptr;
	std::vector<processing_node_output_channel> output_channels_;

protected:
	void verify_connections_validity_() const;

	void handler_setup_();
	void handler_pre_process_(processing_node_job&);
	void handler_process_(processing_node_job&);
	
	processing_node_job begin_job_();
	void finish_job_(processing_node_job&);
	
	virtual node::pull_result output_pull_(time_span& span, bool reconnect) = 0;
	virtual timed_frame_array_view output_begin_read_(time_unit duration) = 0;
	virtual void output_end_read_(time_unit duration) = 0;

public:
	processing_node(graph&, bool with_output);
	~processing_node() override;
	
	void set_handler(processing_node_handler&);
	
	input_type& add_input();
	output_channel_type& add_output_channel();
				
	bool has_output() const { return (outputs_count() > 0); }
	output_type& output() { return output_at(0); }
	const output_type& output() const { return output_at(0); }

	std::size_t output_channels_count() const noexcept;	
	output_channel_type& output_channel_at(std::ptrdiff_t index);
	const output_channel_type& output_channel_at(std::ptrdiff_t index) const;
};


/// Job of \ref processing_node.
/** Exists during processing of one frame, and is created by the \ref processing_node. Contains readable view
 ** with frames from each activated input, and writable multi-channel view for output frame.
 ** Passed to the \ref processing_node_handler. */
class processing_node_job {
private:
	using input_view_handle = std::pair<node_input*, timed_frame_array_view>;

	processing_node& node_;
	std::vector<input_view_handle> input_handles_;
	frame_view output_view_;

	bool end_marked_ = false;
	
	static bool is_null_(const input_view_handle& in) { return (in.first == nullptr); }
	static void set_null_(input_view_handle& in) { in = { nullptr, timed_frame_array_view::null() }; }

public:
	explicit processing_node_job(processing_node& nd);
	processing_node_job(const processing_node_job&) = delete;
	processing_node_job(processing_node_job&&) = default;
	~processing_node_job();

	processing_node_job& operator=(const processing_node_job&) = delete;
	processing_node_job& operator=(processing_node_job&&) = default;
	
	void attach_output_view(const frame_view&);
	void detach_output_view();

	bool begin_input(processing_node_input&);
	void end_input(processing_node_input&);
	void cancel_inputs();
	
	bool end_was_marked() const noexcept { return end_marked_; }
		
	time_unit time() const noexcept { return node_.current_time(); }
	void mark_end() noexcept { end_marked_ = true; }
	
	bool has_input_view(std::ptrdiff_t index) const noexcept;
	const timed_frame_array_view& input_view(std::ptrdiff_t index) const;
	
	bool has_output_view() const noexcept;
	const frame_view& output_view() const;
};



inline processing_node& processing_node_output::this_node() {
	return static_cast<processing_node&>(node_output::this_node());
}

inline const processing_node& processing_node_output::this_node() const {
	return static_cast<const processing_node&>(node_output::this_node());
}


}}

#endif
