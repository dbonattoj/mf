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
#include "../queue/frame.h"
#include "../queue/timed_ring.h"

namespace mf { namespace flow {

/// Output channel of \ref sync_node.
/** Contains the ring buffer with the output frames on that channel. */
class sync_node_output_channel : public processing_node_output_channel {
private:
	std::unique_ptr<timed_ring> ring_;
	
public:
	sync_node_output_channel(processing_node& nd, std::ptrdiff_t index) :
		node_(nd), index_(index) { }
	
	const timed_ring& this_ring() const { return *ring_; }
	timed_ring& this_ring() { return *ring_; }

	void setup(time_unit required_capacity);
	
	frame_view begin_write_frame(time_unit expected_time) override;
	void end_write_frame() override;
	void cancel_write_frame() override;
	
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


/// Synchronous node base class.
/** Processes frames synchronously when pulled from output. */
class sync_node final : public processing_node {
private:
	std::ptrdiff_t lowest_capacity_channel_index_ = -1;

	sync_node_output_channel& output_channel_(std::ptrdiff_t index) {
		return static_cast<sync_node_output_channel&>();
	}

	bool process_next_frame_(time_unit t);	
	time_unit write_start_time_() const;
	time_span readable_time_span_() const;
	void seek_(time_unit t);

public:
	explicit sync_node(graph& gr) : processing_node(gr, true) { }
	
	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
	
	sync_node_output_channel& add_output_channel() override
		{ return add_output_channel_<sync_node_output_channel>(); }

	processing_node_output_channel& output_channel_at(std::ptrdiff_t index)
		{ return static_cast<processing_node_output_channel&>(processing_node::output_channel_at(index)); }
	const processing_node_output_channel& output_channel_at(std::ptrdiff_t index) const
		{ return static_cast<const processing_node_output_channel&>(processing_node::output_channel_at(index)); }
				
	void setup() final override;

	pull_result output_pull_(time_span&, bool reconnected) override;
};




}}

#endif
