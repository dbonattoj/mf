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
