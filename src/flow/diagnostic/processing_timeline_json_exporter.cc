#include "processing_timeline_json_exporter.h"
#include "processing_timeline.h"
#include "../graph.h"
#include "../processing/processing_node.h"
#include "../processing/sync_node.h"
#include "../processing/async_node.h"
#include "../processing/sink_node.h"

namespace mf { namespace flow {

using json = nlohmann::json;

void processing_timeline_json_exporter::compute_earliest_clock_time_() {
	earliest_clock_time_ = timeline_.job_at(0).start_clock_time;
	
	for(std::ptrdiff_t i = 1; i < timeline_.jobs_count(); ++i) {
		const processing_timeline::job& job = timeline_.job_at(i);
		clock_time_type start_clock_time = job.start_clock_time;
		if(start_clock_time < earliest_clock_time_) earliest_clock_time_ = start_clock_time;
	}
}


std::chrono::microseconds processing_timeline_json_exporter::generate_clock_time_(const clock_time_type& ct) const {
	return std::chrono::duration_cast<std::chrono::microseconds>(ct - earliest_clock_time_);
}


json processing_timeline_json_exporter::generate_node_jobs_(const node& nd) const {
	json j_jobs;
	for(std::ptrdiff_t i = 0; i < timeline_.jobs_count(); ++i) {
		const processing_timeline::job& job = timeline_.job_at(i);
		if(&job.node != &nd) continue;
		if(job.end_clock_time == clock_time_type()) continue;
		
		j_jobs.push_back({
			{ "frame_time", job.frame_time },
			{ "start_clock_time", generate_clock_time_(job.start_clock_time).count() },
			{ "end_clock_time", generate_clock_time_(job.end_clock_time).count() }
		});
	}
	return j_jobs;
}


processing_timeline_json_exporter::processing_timeline_json_exporter(const processing_timeline& timeline) :
	timeline_(timeline) { }


void processing_timeline_json_exporter::generate(std::ostream& out) {
	compute_earliest_clock_time_();

	json j_nodes = json::array();

	const graph& gr = timeline_.this_graph();
	for(std::ptrdiff_t i = 0; i < gr.nodes_count(); ++i) {
		const node& nd = gr.node_at(i);
		if(! is_processing_node(nd)) continue;
		
		std::string node_type;
		if(is_sync_node(nd)) node_type = "sync";
		else if(is_async_node(nd)) node_type = "async";
		else if(is_sink_node(nd)) node_type = "sink";
		Assert(! node_type.empty());
		
		j_nodes.push_back({
			{ "id", i },
			{ "name", nd.name() },
			{ "type", node_type },
			{ "jobs", generate_node_jobs_(nd) }
		});
	}
	
	json j = {
		{ "nodes", j_nodes }
	};
	
	out << j;
}

}}
