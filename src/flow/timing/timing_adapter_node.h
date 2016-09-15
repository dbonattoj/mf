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

#ifndef MF_FLOW_TIMING_ADAPTER_NODE_H_
#define MF_FLOW_TIMING_ADAPTER_NODE_H_

#include "../node_derived.h"
#include "../node_input.h"
#include "../node_output.h"
#if 0
namespace mf { namespace flow {

class node_graph;
class timing_adapter_node;

/// Output of \ref timing_adapter_node.
class timing_adapter_node_output final : public node_output {
private:
	timing_adapter_node& this_node();
	const timing_adapter_node& this_node() const;

public:
	explicit timing_adapter_node_output(node& nd) : node_output(nd) { }
	
	std::size_t channels_count() const noexcept override;
	std::string channel_name_at(std::ptrdiff_t i) const override;
	node::pull_result pull(time_span& span, bool reconnect) override;
	node_frame_window_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


/// Node which connects parts of graph with different stream timings.
/** Has one input and one output. */
class timing_adapter_node final : public node_derived<node_input, timing_adapter_node_output> {
	using base = node_derived<node_input, timing_adapter_node_output>;
	
public:
	explicit timing_adapter_node(node_graph&);
	
	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;

	void launch() override;
	void stop() override;
	void pre_setup() override;
	void setup() override;
};


inline timing_adapter_node& timing_adapter_node_output::this_node() {
	return static_cast<timing_adapter_node&>(node_output::this_node());
}

inline const timing_adapter_node& timing_adapter_node_output::this_node() const {
	return static_cast<const timing_adapter_node&>(node_output::this_node());
}

}}

#endif
#endif
