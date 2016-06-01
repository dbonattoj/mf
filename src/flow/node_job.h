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

#ifndef MF_FLOW_NODE_JOB_H_
#define MF_FLOW_NODE_JOB_H_

#include "node.h"
#include <vector>
#include <utility>

namespace mf { namespace flow {

/// Work unit of flow graph node.
/** Contains input and output views for concrete node to read/write a frame to. Handles cast from internel genetic views
 ** to ndarray views of specified dimension and element type. */
class node_job {
private:
	using input_view_handle_type = std::pair<node_input*, timed_frame_array_view>;
	using output_view_handle_type = std::pair<node_output*, frame_view>;

	node& node_;
	time_unit time_ = -1;
	bool end_marked_ = false;
	
	std::vector<input_view_handle_type> inputs_stack_;
	std::vector<input_view_handle_type*> inputs_map_; 

	std::vector<output_view_handle_type> outputs_stack_;
	std::vector<output_view_handle_type*> outputs_map_; 

public:
	explicit node_job(node&);
	~node_job();
	
	/// \name Interface for set up.
	///@{
	void define_time(time_unit t);
	void push_input(node_input&, const timed_frame_array_view&);
	void push_output(node_output&, const frame_view&);
	node_input* pop_input();
	node_output* pop_output();
	bool end_was_marked() const noexcept { return end_marked_; }
	///@}
	
	time_unit time() const noexcept { return time_; }
	void mark_end() { end_marked_ = true; }

	template<typename Input> decltype(auto) in_full(Input&);
	template<typename Input> decltype(auto) in(Input&);
	template<typename Output> decltype(auto) out(Output&);
	template<typename Param> decltype(auto) param(const Param&);
};

}}

#include "node_job.tcc"

#endif
