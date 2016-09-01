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

#include "processing_timeline.h"
#include "../graph.h"
#include "../processing_node.h"
#include <ostream>

namespace mf { namespace flow {

processing_timeline::processing_timeline(graph& gr) :
	graph_(gr) { }


void processing_timeline::processing_node_job_started(const processing_node& nd, time_unit t) {
	clock_time_type start_clock_time = clock_type::now();
	std::lock_guard<std::mutex> lock(jobs_mutex_);
	
	jobs_.push_back({ nd, t, start_clock_time, clock_time_type() });

	current_node_jobs_.emplace(&nd, &jobs_.back());
}


void processing_timeline::processing_node_job_finished(const processing_node& nd, time_unit t) {
	clock_time_type end_clock_time = clock_type::now();
	std::lock_guard<std::mutex> lock(jobs_mutex_);
	
	auto it = current_node_jobs_.find(&nd);
	Assert(it != current_node_jobs_.end());
	
	it->second->end_clock_time = end_clock_time;
	
	current_node_jobs_.erase(it);
}


void processing_timeline::launched(const graph& gr) {
	
}


void processing_timeline::stopped(const graph& gr) {
	
}


}}
