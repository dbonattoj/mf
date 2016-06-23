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

#ifndef MF_FLOW_SINK_NODE_H_
#define MF_FLOW_SINK_NODE_H_

#include "filter_node.h"

namespace mf { namespace flow {

class graph;

/// Sink node base class.
/** Has one of multiple inputs and no outputs. There is one sink node per graph. Controls time flow of graph. */
class sink_node final : public filter_node {
public:	
	explicit sink_node(graph& gr) : filter_node(gr) { }
	
	void setup() final override;
	void launch() final override;
	void stop() final override;
	bool process_next_frame() final override;
	
	void setup_graph();
	
	void pull(time_unit t);
	void pull_next_frame() { process_next_frame(); }
	
	void seek(time_unit t);
	
	node_input& add_input(time_unit past_window, time_unit future_window) override {
		return add_input_<node_input>(past_window, future_window);
	}
	
	node_output& add_output(const frame_format& format) override { throw 0; }
};


}}

#endif
