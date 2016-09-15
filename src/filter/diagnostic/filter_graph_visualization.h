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

