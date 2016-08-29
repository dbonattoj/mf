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
