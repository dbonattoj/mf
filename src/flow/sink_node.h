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

namespace mf { namespace flow {

class graph;

/// Sink node base class.
/** Has one of multiple inputs and no outputs. There is one sink node per graph. Controls time flow of graph. */
class sink_node final : public processing_node {
protected:
	pull_result output_pull_(time_span&, bool reconnected) override { throw 0; }
	timed_frame_array_view output_begin_read_(time_unit duration) override { throw 0; }
	void output_end_read_(time_unit duration) override { throw 0; }

public:	
	explicit sink_node(graph& gr) : processing_node(gr) { }
	
	time_unit minimal_offset_to(const node&) const override { return 0; }
	time_unit maximal_offset_to(const node&) const override { return 0; }
	
	void setup() final override;
	
	void setup_graph();
	
	bool process_next_frame();
	void pull(time_unit t);
	void pull_next_frame() { process_next_frame(); }
	
	void seek(time_unit t);
};


}}

#endif
