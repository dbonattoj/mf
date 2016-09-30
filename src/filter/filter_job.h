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

#ifndef MF_FLOW_FILTER_JOB_H_
#define MF_FLOW_FILTER_JOB_H_

#include "../flow/processing/processing_node.h"

namespace mf { namespace flow {

template<std::size_t Dim, typename Elem> class filter_input;
template<std::size_t Dim, typename Elem> class filter_output;
template<typename Value> class filter_parameter;

class filter_job {
private:
	processing_node_job& node_job_;

public:
	explicit filter_job(processing_node_job& job) : node_job_(job) { }
	
	time_unit time() const noexcept { return node_job_.time(); }
	void mark_end() noexcept { node_job_.mark_end_of_stream(); }

	/// Input view.
	///@{
	template<std::size_t Dim, typename Elem>
	ndarray_timed_view<Dim + 1, Elem> in_full(filter_input<Dim, Elem>&);
	
	template<std::size_t Dim, typename Elem>
	ndarray_view<Dim, Elem> in(filter_input<Dim, Elem>&);
	
	template<std::size_t Dim, typename Elem>
	ndarray_view<Dim, Elem> in(filter_input<Dim, Elem>&, time_unit t);
	///@}


	/// Output view.
	///@{
	template<std::size_t Dim, typename Elem>
	ndarray_view<Dim, Elem> out(filter_output<Dim, Elem>&);
	///@}


	/// De-activate inputs.
	///@{
	void set_activated(filter_input_base&, bool activated);
	bool is_activated(const filter_input_base&);
	
	void activate(filter_input_base& in) { set_activated(in, true); }
	void deactivate(filter_input_base& in) { set_activated(in, false); }
	///@}
	
	
	/// Access parameters (own & input reference)
	///@{
	template<typename Value>
	Value param(const filter_parameter<Value>&);

	template<typename Value>
	Value param(const filter_parameter<Value>&, time_unit t);	
	///@}
	
	
	/// Set own parameters.
	///@{
	template<typename Value>
	void set_param(const filter_parameter<Value>&, const Value&);
	///@}
	
	
	/// Send reference parameters.
	///@{
	template<typename Value>
	void send_param(const filter_parameter<Value>&, const Value&);
	///@}
};

}}

#include "filter_job.tcc"

#endif
