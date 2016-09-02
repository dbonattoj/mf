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

#ifndef MF_FLOW_SYNC_NODE_H_
#define MF_FLOW_SYNC_NODE_H_

#include <memory>
#include "processing_node.h"
#include "processing_node_job.h"
#include "../../queue/frame.h"
#include "../../queue/timed_ring.h"

namespace mf { namespace flow {

/// Synchronous node base class.
/** Processes frames synchronously when pulled from output. Can have multiple inputs, but only one output. */
class sync_node final : public processing_node {
private:
	std::unique_ptr<timed_ring> ring_;

	bool process_next_frame_();

public:
	explicit sync_node(graph&);
	
	thread_index processing_thread_index() const override;
		
	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
	
	void setup() final override;
			
	pull_result output_pull_(time_span&, bool reconnected) override;
	timed_frame_array_view output_begin_read_(time_unit duration) override;
	void output_end_read_(time_unit duration) override;
};


inline bool is_sync_node(const node& nd) {
	return (dynamic_cast<const sync_node*>(&nd) != nullptr);
}


}}

#endif
