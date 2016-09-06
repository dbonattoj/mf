#ifndef MF_PROCESSING_TIMELINE_JSON_EXPORTER_H_
#define MF_PROCESSING_TIMELINE_JSON_EXPORTER_H_

#include "processing_timeline.h"
#include <ostream>
#include <json.hpp>

namespace mf { namespace flow {
	
class node;

class processing_timeline_json_exporter {
private:
	using clock_time_type = processing_timeline::clock_time_type;

	const processing_timeline& timeline_;
	
	clock_time_type earliest_clock_time_;
	clock_time_type latest_clock_time_;
	
	void compute_clock_time_bounds_();
	nlohmann::json generate_node_jobs_(const node&) const;
	std::chrono::microseconds generate_clock_time_(const clock_time_type&) const;

public:
	explicit processing_timeline_json_exporter(const processing_timeline&);
	
	void generate(std::ostream&);
};

}}

#endif
