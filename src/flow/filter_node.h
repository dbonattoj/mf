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

#ifndef MF_FLOW_FILTER_NODE_H_
#define MF_FLOW_FILTER_NODE_H_

#include "node.h"
#include <memory>
#include <utility>

namespace mf { namespace flow {

class filter;
class graph;
class node_job;

/// Node which delegates concrete frame processing to associated \ref filter object.
class filter_node : public node {
private:
	std::unique_ptr<filter> filter_;

protected:
	void setup_filter();
	void pre_process_filter(node_job&);
	void process_filter(node_job&);
	

public:
	filter_node(graph& gr);
	~filter_node();
	
	template<typename Filter, typename... Args>
	Filter& set_filter(Args&&... args) {
		Filter* filter = new Filter(*this, std::forward<Args>(args)...);
		filter_.reset(filter);
		return *filter;
	}
	
	filter& this_filter() { return *filter_; }
	const filter& this_filter() const { return *filter_; }
	
	virtual node_input& add_input(time_unit past_window, time_unit future_window) = 0;
	virtual node_output& add_output(const frame_format& format) = 0;
};

}}

#endif
