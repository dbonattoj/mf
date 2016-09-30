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

#include "processing_node.h"
#include "processing_node_job.h"

namespace mf { namespace flow {

class node_graph;

/// Sink node base class.
/** Has one of multiple inputs and no outputs. There is one sink node in the graph. Controls time flow of graph. */
class sink_node final : public processing_node {
protected:
	void output_pre_pull_(const time_span&) override { throw 0; }
	pull_result output_pull_(time_span&) override { throw 0; }
	node_frame_window_view output_begin_read_(time_unit duration) override { throw 0; }
	void output_end_read_(time_unit duration) override { throw 0; }

public:	
	explicit sink_node(node_graph&);
	
	thread_index processing_thread_index() const override;
	
	time_unit minimal_offset_to(const node&) const override { return 0; }
	time_unit maximal_offset_to(const node&) const override { return 0; }
	
	void setup() final override;
	
	void setup_graph();
	
	pull_result pull(time_unit t);
	pull_result pull_next_frame();
	
	void seek(time_unit t);
};


inline bool is_sink_node(const node& nd) {
	return (dynamic_cast<const sink_node*>(&nd) != nullptr);
}

}}

#endif
