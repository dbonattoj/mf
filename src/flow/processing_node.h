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


class processing_node_output : public node_output {
private:
	processing_node& this_node();
	const processing_node& this_node() const;	
	
public:
	using node_output::node_output;

	node::pull_result pull(time_span& span, bool reconnect) override;
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


class processing_node_handler {
public:
	virtual void handler_setup() = 0;
	virtual void handler_pre_process(processing_node_job&) = 0;
	virtual void handler_process(processing_node_job&) = 0;
};


class processing_node : public node {
	friend class processing_node_output;

private:
	processing_node_handler* handler_ = nullptr;

protected:
	void handler_setup_();
	void handler_pre_process_(processing_node_job&);
	void handler_process_(processing_node_job&);
	
	processing_node_job begin_job_();
	void finish_job_(processing_node_job&);

	virtual pull_result output_pull_(time_span&, bool reconnected) = 0;
	virtual timed_frame_array_view output_begin_read_(time_unit duration) = 0;
	virtual void output_end_read_(time_unit duration) = 0;
	
public:
	using output_type = processing_node_output;

	explicit processing_node(graph&);
	~processing_node() override;
	
	void set_handler(processing_node_handler&);
	
	processing_node_output& output();
	const processing_node_output& output() const;
				
	node_input& add_input();
	processing_node_output& add_output();
};



inline processing_node_output& processing_node::output() {
	Expects(outputs().size() == 1);
	return static_cast<processing_node_output&>(*outputs().front());
}

inline const processing_node_output& processing_node::output() const {
	Expects(outputs().size() == 1);
	return static_cast<const processing_node_output&>(*outputs().front());
}

inline processing_node& processing_node_output::this_node() {
	return static_cast<processing_node&>(node_output::this_node());
}

inline const processing_node& processing_node_output::this_node() const {
	return static_cast<const processing_node&>(node_output::this_node());
}	



class processing_node_job {
private:
	using input_view_handle = std::pair<node_input*, timed_frame_array_view>;

	time_unit time_ = -1;
	std::vector<input_view_handle> inputs_stack_;
	std::vector<input_view_handle*> inputs_slots_;

	frame_view output_view_;
	bool end_marked_ = false;

public:
	explicit processing_node_job(processing_node& nd);
	~processing_node_job();
	
	void attach_output(const frame_view& output_view);
	void detach_output();
	bool has_output() const { return ! output_view_.is_null(); }
	
	bool push_input(node_input&);
	node_input& pop_input();
	bool has_inputs() const { return (inputs_stack_.size() > 0); }
	void cancel_inputs();
		
	time_unit time() const noexcept { return time_; }
	void mark_end() noexcept { end_marked_ = true; }
	
	bool has_input_view(std::ptrdiff_t index) const noexcept;
	const timed_frame_array_view& input_view(std::ptrdiff_t index);
	const frame_view& output_view();
	bool end_was_marked() const noexcept { return end_marked_; }
};


}}

#endif
