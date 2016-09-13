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
