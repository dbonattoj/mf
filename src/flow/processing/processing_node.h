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

#include "../node_derived.h"
#include "../node_input.h"
#include "../node_output.h"
#include "../../nd/ndarray_format.h"
#include "../node_frame_format.h"
#include <memory>
#include <utility>
#include <vector>

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



/// Channel of the \ref processing_node_output of a \ref processing_node.
/** Defines \ref ndarray_format for the frames on that channel.
 ** Has index value for use with \ref processing_node_job. */
class processing_node_output_channel final {
private:
	processing_node& node_;
	std::string name_;
	const std::ptrdiff_t index_;
	ndarray_format frame_format_;

public:
	processing_node_output_channel(processing_node& nd, std::ptrdiff_t index) :
		node_(nd), index_(index) { }
	
	const std::string& name() const { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }

	processing_node& this_node() { return node_; }
	const processing_node& this_node() const { return node_; }
	std::ptrdiff_t index() const { return index_; }
	
	void define_frame_format(const ndarray_format& frm) { frame_format_ = frm; }
	const ndarray_format& frame_format() const noexcept { return frame_format_; }
};



/// Output of \ref processing_node.
/** The \ref processing_node has no or one output. Its implementation is delegated to \ref processing_node. */
class processing_node_output final : public node_output {
private:
	processing_node& this_node();
	const processing_node& this_node() const;	

protected:
	void added_propagated_parameter_(parameter_id, const node_input& source) override;

public:
	using node_output::node_output;
	
	std::size_t channels_count() const noexcept override;
	std::string channel_name_at(std::ptrdiff_t i) const override;
	
	node::pull_result pull(time_span& span, bool reconnect) override;
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};



/// Input of \ref processing_node.
/** Has index value for use with \ref processing_node_job. */
class processing_node_input final : public node_input {
private:	
	const std::ptrdiff_t index_;

public:
	using node_input::node_input;

	processing_node_input(processing_node&, std::ptrdiff_t index);
	
	thread_index reader_thread_index() const final override;
	
	std::ptrdiff_t index() const { return index_; }
};


/// Node which processes frames one-by-one, base class.
/** Frame processing is delegated to external \ref processing_node_handler, for example \ref filter.
 ** Has no or one output, which can have multiple channels. Can have any number of inputs.
 ** Creates \ref processing_node_job for each frame to process, with indexed input and output views. */
class processing_node : public node_derived<processing_node_input, processing_node_output> {
	using base = node_derived<processing_node_input, processing_node_output>;
	friend class processing_node_output;

public:
	using output_channel_type = processing_node_output_channel;
	using input_index_type = std::ptrdiff_t;

private:
	processing_node_handler* handler_ = nullptr;
	std::vector<std::unique_ptr<processing_node_output_channel>> output_channels_;
	std::multimap<parameter_id, input_index_type> propagated_parameters_guide_;

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

	node_frame_format output_frame_format_() const;

public:
	processing_node(node_graph&, bool with_output);
	~processing_node() override;
	
	void set_handler(processing_node_handler&);

	virtual thread_index processing_thread_index() const = 0;
	
	input_type& add_input();
	output_channel_type& add_output_channel();
				
	bool has_output() const { return (outputs_count() > 0); }
	output_type& output() { return output_at(0); }
	const output_type& output() const { return output_at(0); }

	std::size_t output_channels_count() const { return output_channels_.size(); }
	output_channel_type& output_channel_at(std::ptrdiff_t index);
	const output_channel_type& output_channel_at(std::ptrdiff_t index) const;
	
	std::vector<input_index_type> propagated_parameters_inputs(parameter_id) const;
};



inline processing_node& processing_node_output::this_node() {
	return static_cast<processing_node&>(node_output::this_node());
}

inline const processing_node& processing_node_output::this_node() const {
	return static_cast<const processing_node&>(node_output::this_node());
}

inline thread_index processing_node_input::reader_thread_index() const {
	return static_cast<const processing_node&>(this_node()).processing_thread_index();
}


inline bool is_processing_node(const node& nd) {
	return (dynamic_cast<const processing_node*>(&nd) != nullptr);
}



}}

#endif
