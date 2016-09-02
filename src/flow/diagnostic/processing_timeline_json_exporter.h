#ifndef MF_PROCESSING_TIMELINE_JSON_EXPORTER_H_
#define MF_PROCESSING_TIMELINE_JSON_EXPORTER_H_

#include <ostream>

namespace mf { namespace flow {

class processing_timeline_json_exporter {
private:
	using clock_time_type = processing_timeline::clock_time_type;

	const processing_timeline& timeline_;
	std::ostream& output_;
	
	clock_time_type earliest_clock_time_;

	clock_time_type find_earliest_clock_time_() const;
	

public:
	processing_timeline_json_exporter(const processing_timeline&, std::ostream&);
	
	void generate();
};

}}

#endif
