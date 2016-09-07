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

#ifndef MF_FLOW_NODE_OUTPUT_H_
#define MF_FLOW_NODE_OUTPUT_H_

#include "node.h"
#include "parameter/node_parameter.h"

namespace mf { namespace flow {


/// Output port of node in node graph.
/** One output port have multiple *channels*. The output is *pulled* as a whole, but data is read from individual
 ** channels. The channels may have different formats. */
class node_output {
private:
	node& node_;
	std::string name_ = "out";

	node_input* connected_input_ = nullptr;
	
	std::vector<parameter_id> output_parameters_;
	
protected:
	node_output(const node_output&) = delete;
	node_output& operator=(const node_output&) = delete;

public:	
	explicit node_output(node& nd);
	virtual ~node_output() = default;

	node& this_node() const noexcept { return node_; }
	
	bool needs_output_parameter(parameter_id);
	
	void add_output_parameter(parameter_id);

	bool has_output_parameter(parameter_id) const;	
	std::size_t output_parameters_count() const { return output_parameters_.size(); }
	parameter_id output_parameter_at(std::ptrdiff_t i) const { return output_parameters_.at(i); }

	thread_index reader_thread_index() const;

	const std::string& name() const { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }

	bool is_connected() const noexcept { return (connected_input_ != nullptr); }
	node_input& connected_input() const noexcept { Assert(is_connected()); return *connected_input_; }
	node& connected_node() const noexcept;
	
	void input_has_connected(node_input&);
	void input_has_disconnected();
	
	time_unit end_time() const noexcept;

	virtual std::size_t channels_count() const noexcept = 0;
	virtual std::string channel_name_at(std::ptrdiff_t i) const = 0;
	virtual node::pull_result pull(time_span& span, bool reconnect) = 0;
	virtual timed_frame_array_view begin_read(time_unit duration) = 0;
	virtual void end_read(time_unit duration) = 0;

	bool is_online() const;
};


}}

#endif
