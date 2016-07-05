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

#ifndef MF_FLOW_FILTER_NODE_H_
#define MF_FLOW_FILTER_NODE_H_

#include "node.h"
#include <memory>
#include <utility>

namespace mf { namespace flow {

class graph;
class filter;
class filter_node;
class filter_node_job;


class filter_node_output : public node_output {
private:
	filter_node& this_node();
	const filter_node& this_node() const;	
	
public:
	using node_output::node_output;

	node::pull_result pull(time_span& span, bool reconnect) override;
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


/// Node containing a filter, base class.
class filter_node : public node {
	friend class filter_node_output;

private:
	filter* filter_ = nullptr;

protected:
	void setup_filter_();
	void pre_process_filter_(filter_node_job&);
	void process_filter_(filter_node_job&);
	
	filter_node_job begin_job_();
	void finish_job_(filter_node_job&);

	virtual pull_result output_pull_(time_span&, bool reconnected) = 0;
	virtual timed_frame_array_view output_begin_read_(time_unit duration) = 0;
	virtual void output_end_read_(time_unit duration) = 0;

public:
	using output_type = filter_node_output;

	explicit filter_node(graph&);
	~filter_node() override;
	
	filter_node_output& output() { return static_cast<filter_node_output&>(*outputs().front()); }
	const filter_node_output& output() const { return static_cast<const filter_node_output&>(*outputs().front()); }
	
	filter& this_filter() { return *filter_; }
	const filter& this_filter() const { return *filter_; }
	
	void set_filter(filter& filt);
		
	node_input& add_input();
	node_output& add_output();
};


inline filter_node& filter_node_output::this_node() {
	return static_cast<filter_node&>(node_output::this_node());
}

inline const filter_node& filter_node_output::this_node() const {
	return static_cast<const filter_node&>(node_output::this_node());
}	



class filter_node_job {
private:
	using input_view_handle = std::pair<node_input*, timed_frame_array_view>;

	time_unit time_ = -1;
	std::vector<input_view_handle> inputs_stack_;
	std::vector<input_view_handle*> inputs_slots_;
bool det=true;
	frame_view output_view_;
	bool end_marked_ = false;

public:
	explicit filter_node_job(filter_node& nd);
	~filter_node_job();
	
	void attach_output(const frame_view& output_view);
	void detach_output();
	bool has_output() const { return !det;/*return ! output_view_.is_null();*/ }
	
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
