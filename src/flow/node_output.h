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

namespace mf { namespace flow {

class node_input;
class node_parameter_relay;

/// Output port of node in node graph.
/** One output port have multiple *channels*. The output is *pulled* as a whole, but data is read from individual
 ** channels. The channels may have different formats. */
class node_output {
private:
	node& node_;
	std::string name_ = "out";

	node_input* connected_input_ = nullptr;
	
	/// Parameters which are added to frames passing through this output.
	/** Part of satellite data of frames. Parameter values will be copied from node input(s). Node must ensure that at
	 ** least these parameters are present in the output frames, except when they are not available (e.g. when an
	 ** input is disabled). */
	std::vector<parameter_id> propagated_parameters_;
	
protected:
	node_output(const node_output&) = delete;
	node_output& operator=(const node_output&) = delete;
	
public:	
	explicit node_output(node& nd);
	virtual ~node_output() = default;

	node& this_node() const noexcept { return node_; }
	
	bool add_propagated_parameter_if_needed(parameter_id);
	bool add_relayed_parameter_if_needed(parameter_id, const node_parameter_relay& preceding_relay);
	
	bool has_propagated_parameter(parameter_id) const;
	std::size_t propagated_parameters_count() const { return propagated_parameters_.size(); }
	parameter_id propagated_parameter_at(std::ptrdiff_t i) const { return propagated_parameters_.at(i); }

	thread_index reader_thread_index() const;

	const std::string& name() const { return name_; }
	void set_name(const std::string& nm) { name_ = nm; }

	bool is_connected() const noexcept { return (connected_input_ != nullptr); }
	node_input& connected_input() const noexcept { Assert(is_connected()); return *connected_input_; }
	node& connected_node() const noexcept;
	
	void input_has_connected(node_input&);
	void input_has_disconnected();
	
	virtual std::size_t channels_count() const noexcept = 0;
	virtual std::string channel_name_at(std::ptrdiff_t i) const = 0;
	virtual void pre_pull(const time_span& span) = 0;
	virtual node::pull_result pull(time_span& span) = 0;
	virtual node_frame_window_view begin_read(time_unit duration) = 0;
	virtual void end_read(time_unit duration) = 0;
};


}}

#endif
