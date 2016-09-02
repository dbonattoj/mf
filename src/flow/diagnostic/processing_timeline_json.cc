#include "processing_timeline_json.h"
#include "processing_timeline.h"
#include "../graph.h"
#include "../processing/processing_node.h"
#include "../processing/sync_node.h"
#include "../processing/async_node.h"
#include "../processing/sink_node.h"
#include <json.hpp>
#include <ostream>
#include <istream>

namespace mf { namespace flow {

using json = nlohmann::json;

namespace {
	json node_jobs_(const processing_timeline& timeline, const node& nd) {
		json j_jobs;
		for(std::ptrdiff_t i = 0; i < timeline.jobs_count(); ++i) {
			const processing_timeline::job& job = timeline.job_at(i);
			if(&job.node != &nd) continue;
			
			json j_job = {
				{ "frame_time", job.frame_time },
				{ "start_clock_time",  }
				{ "end_clock_time" }
			};
		}
	}
}

void export_processing_timeline_to_json(const processing_timeline& timeline, std::ostream& str) {
	const graph& gr = timeline.this_graph();
	for(std::ptrdiff_t i = 0; i < gr.nodes_count(); ++i) {
		const node& nd = gr.node_at(i);
		if(! is_processing_node(nd)) continue;
		
		std::string node_type;
		if(is_sync_node(nd)) node_type = "sync";
		else if(is_async_node(nd)) node_type = "async";
		else if(is_sink_node(nd)) node_type = "sink";
		Assert(node_type);
		
		json j_node = {
			{ "id", i },
			{ "name", nd.name() },
			{ "type", node_type }
		};
		
		json j_jobs = json::array();
		
	}
}


processing_timeline import_processing_timeline_from_json(std::istream& str) {
	
}

}}
