#ifndef MF_FLOW_FILTER_GRAPH_VISUALIZATION_H_
#define MF_FLOW_FILTER_GRAPH_VISUALIZATION_H_

#include <iosfwd>
#include <string>
#include "../../utility/uid_pool.h"

namespace mf { namespace flow {

class filter_graph;
class filter;

class filter_graph_visualization {
private:	
	std::ostream& output_;
	const filter_graph& graph_;
	std::string graph_id_ = "G";
	uid_pool uid_pool_;

	bool with_parameters_ = true;
	
	template<typename T> std::string uid_(const T& object, const std::string& prefix);
	
	void generate_filter_(const filter&);
	void generate_filter_input_connections_(const filter&);
	void generate_ranks_();

public:
	filter_graph_visualization(const filter_graph&, std::ostream&);
	
	void generate();
};


void export_filter_graph_visualization(const filter_graph&, const std::string& filename);


}}

#endif

