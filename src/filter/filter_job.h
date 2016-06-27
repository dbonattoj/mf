#ifndef MF_FLOW_FILTER_JOB_H_
#define MF_FLOW_FILTER_JOB_H_

namespace mf { namespace flow {

class filter_job {
private:
	filter_node_job& node_job_;

public:
	filter_job(filter_node_job& job) : node_job_(job) { }
	
	time_unit time() const noexcept { return node_job_.time(); }
	void mark_end() noexcept { node_job_.mark_end(); }

	template<typename Input> decltype(auto) in_full(Input&);
	template<typename Input> decltype(auto) in(Input&);
	template<typename Output> decltype(auto) out(Output&);
};

}}

#include "filter_job.tcc"

#endif
