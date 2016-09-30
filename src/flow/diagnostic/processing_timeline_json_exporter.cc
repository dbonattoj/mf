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

#include "processing_timeline_json_exporter.h"
#include "processing_timeline.h"
#include "../node_graph.h"
#include "../processing/processing_node.h"
#include "../processing/sync_node.h"
#include "../processing/async_node.h"
#include "../processing/sink_node.h"

namespace mf { namespace flow {

using json = nlohmann::json;

void processing_timeline_json_exporter::compute_clock_time_bounds_() {
	earliest_clock_time_ = timeline_.job_at(0).start_clock_time;
	latest_clock_time_ = timeline_.job_at(0).end_clock_time;
	
	for(std::ptrdiff_t i = 1; i < timeline_.jobs_count(); ++i) {
		const processing_timeline::job& job = timeline_.job_at(i);
		if(job.start_clock_time < earliest_clock_time_) earliest_clock_time_ = job.start_clock_time;
		if(job.end_clock_time > latest_clock_time_) latest_clock_time_ = job.end_clock_time;
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
			{ "t", job.frame_time },
			{ "from", generate_clock_time_(job.start_clock_time).count() },
			{ "to", generate_clock_time_(job.end_clock_time).count() }
		});
	}
	return j_jobs;
}


processing_timeline_json_exporter::processing_timeline_json_exporter(const processing_timeline& timeline) :
	timeline_(timeline) { }


void processing_timeline_json_exporter::generate(std::ostream& out) {
	if(timeline_.jobs_count() > 0) compute_clock_time_bounds_();

	json j_nodes = json::array();

	const node_graph& gr = timeline_.graph();
	for(std::ptrdiff_t i = 0; i < gr.nodes_count(); ++i) {
		const node& nd = gr.node_at(i);
		if(! is_processing_node(nd)) continue;
		
		std::string node_type;
		if(is_sync_node(nd)) node_type = "sync";
		//else if(is_async_node(nd)) node_type = "async";
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
		{ "nodes", j_nodes },
		{ "duration", generate_clock_time_(latest_clock_time_).count() }
	};
	
	out << j;
}

}}
