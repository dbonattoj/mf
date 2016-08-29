#ifndef MF_FLOW_PROCESSING_TIMELINE_H_
#define MF_FLOW_PROCESSING_TIMELINE_H_

#include "diagnostic_handler.h"
#include <chrono>
#include <vector>
#include <map>
#include <mutex>

namespace mf { namespace flow {

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
	graph& graph_;
	std::vector<job> jobs_;
	std::map<const processing_node*, job*> current_node_jobs_;
	std::mutex jobs_mutex_;
	
public:
	explicit processing_timeline(graph&);

	void processing_node_job_started(const processing_node&, time_unit t) override;
	void processing_node_job_finished(const processing_node&, time_unit t) override;
	void launched(const graph&) override;
	void stopped(const graph&) override;
	
	graph& this_graph() { return graph_; }
	const graph& this_graph() const { return graph_; }
	
	std::size_t jobs_count() const { return jobs_.size(); }
	const job& job_at(std::ptrdiff_t index) const { return jobs_.at(index); }
};

}}

#endif
