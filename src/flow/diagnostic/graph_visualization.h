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

#ifndef MF_FLOW_GRAPH_VISUALIZATION_H_
#define MF_FLOW_GRAPH_VISUALIZATION_H_

#include <iosfwd>
#include <string>
#include <map>
#include <cstdint>
#include "../node.h"

namespace mf { namespace flow {

class node_graph;
class node;
class node_input;
class node_output;
class processing_node;
class multiplex_node;

class graph_visualization {
private:	
	std::ostream& output_;
	const node_graph& graph_;
	std::string graph_id_ = "G";
	std::map<std::uintptr_t, std::string> uids_;

	bool thread_index_colors_ = true;
	bool with_state_ = true;
	bool with_parameters_ = true;
	
	template<typename T> std::string uid_(const T& object, const std::string& prefix);
	
	void generate_node_dispatch_(const node&);
	void generate_processing_node_(const processing_node&, bool async, bool sink);
	void generate_multiplex_node_(const multiplex_node&);
	void generate_node_input_connections_(const node&);
	void generate_ranks_();

	std::string thread_index_color_(thread_index) const;

public:
	graph_visualization(const node_graph&, std::ostream&);
	
	void generate();
};


void export_graph_visualization(const node_graph&, const std::string& filename);


template<typename T>
std::string graph_visualization::uid_(const T& object, const std::string& prefix) {
	std::uintptr_t address = reinterpret_cast<std::uintptr_t>(&object);
	auto it = uids_.find(address);
	if(it != uids_.end()) {
		return it->second;
	} else {
		std::size_t index = uids_.size();
		std::string uid = prefix + std::to_string(index);
		uids_.emplace(address, uid);
		return uid;
	}
}

}}

#endif
