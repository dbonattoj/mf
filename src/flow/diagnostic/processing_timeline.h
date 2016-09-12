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

#ifndef MF_FLOW_PROCESSING_TIMELINE_H_
#define MF_FLOW_PROCESSING_TIMELINE_H_

#include "diagnostic_handler.h"
#include <chrono>
#include <vector>
#include <map>
#include <mutex>

namespace mf { namespace flow {

class node_graph;

class processing_timeline : public diagnostic_handler {
public:
	using clock_type = std::chrono::high_resolution_clock;
	using clock_time_type = clock_type::time_point;

	struct job {
		const processing_node& node;
		time_unit frame_time;
		clock_time_type start_clock_time;
		clock_time_type end_clock_time;
	};
	
private:
	node_graph& graph_;
	std::vector<job> jobs_;
	std::map<const processing_node*, job*> current_node_jobs_;
	std::mutex jobs_mutex_;
	
public:
	explicit processing_timeline(node_graph&);

	void processing_node_job_started(const processing_node&, time_unit t) override;
	void processing_node_job_finished(const processing_node&, time_unit t) override;
	void launched(const node_graph&) override;
	void stopped(const node_graph&) override;
	
	node_graph& graph() { return graph_; }
	const node_graph& graph() const { return graph_; }
	
	std::size_t jobs_count() const { return jobs_.size(); }
	const job& job_at(std::ptrdiff_t index) const { return jobs_.at(index); }
};

}}

#endif
