#ifndef MF_FLOW_FILTER_GRAPH_H_
#define MF_FLOW_FILTER_GRAPH_H_

#include "../flow/graph.h"
#include "filter.h"
#include <vector>
#include <memory>
#include <utility>

namespace mf { namespace flow {

class filter_graph {
private:
	std::vector<std::unique_ptr<filter>> filters_;
	std::unique_ptr<graph> node_graph_;
	
public:
	filter_graph() = default;
	filter_graph(const filter_graph&) = delete;
	filter_graph& operator=(const filter_graph&) = delete;
	~filter_graph();

	template<typename Filter, typename... Args>
	Filter& add_filter(Args&&... args) {
		static_assert(std::is_base_of<filter, Node>::value, "filter must be derived class from `filter`");
		Expects(! was_setup());
		filters_.emplace_back(new Filter(std::forward<Args>(args)...));
		return *filters_.back();
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
