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

#ifndef MF_FLOW_FILTER_GRAPH_H_
#define MF_FLOW_FILTER_GRAPH_H_

#include "../flow/graph.h"
#include "../flow/parameter/parameter.h"
#include "filter.h"
#include <vector>
#include <memory>
#include <utility>

namespace mf { namespace flow {

class filter_graph {
public:
	std::vector<std::unique_ptr<filter>> filters_;
	std::unique_ptr<graph> node_graph_;
	
	parameter_id last_parameter_id_ = 0;
	
public:
	filter_graph() = default;
	filter_graph(const filter_graph&) = delete;
	filter_graph& operator=(const filter_graph&) = delete;
	~filter_graph();
	
	parameter_id new_parameter_id();

	template<typename Filter, typename... Args>
	Filter& add_filter(Args&&... args) {
		static_assert(std::is_base_of<filter, Filter>::value, "filter must be derived class from `filter`");
		Expects(! was_setup());
		Filter* filt = new Filter(std::forward<Args>(args)...);
		filters_.emplace_back(filt);
		return *filt;
	}
	
	bool was_setup() const { return (node_graph_ != nullptr); }
	void setup();

	time_unit current_time() const;
	
	void run_until(time_unit last_frame);
	void run_for(time_unit duration);
	bool run();

	void seek(time_unit target_time);
};


}}

#endif
