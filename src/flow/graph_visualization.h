#ifndef MF_FLOW_GRAPH_VISUALIZATION_H_
#define MF_FLOW_GRAPH_VISUALIZATION_H_

#include <iosfwd>
#include <string>

namespace mf { namespace flow {

class graph;
class node;
class node_input;
class node_output;
class processing_node;
class multiplex_node;

class graph_visualization {
private:	
	std::ostream& output_;
	const graph& graph_;
	std::string graph_id_ = "G";
	
	void generate_node_dispatch_(const node&);
	void generate_processing_node_(const processing_node&, bool async, bool sink);
	void generate_multiplex_node_(const multiplex_node&);
	void generate_node_input_connections_(const node&);
	void generate_ranks_();

public:
	graph_visualization(const graph&, std::ostream&);
	
	void generate();
};

void export_graph_visualization(const graph&, const std::string& filename);

}}

#endif
