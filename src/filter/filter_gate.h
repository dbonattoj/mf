#ifndef MF_FLOW_FILTER_GATE_H_
#define MF_FLOW_FILTER_GATE_H_

#include "filter.h"

namespace mf { namespace flow {

class filter_gate_base {
public:
	virtual ~filter_gate_base() = default;
};


template<std::size_t Dim, typename Elem>
class filter_gate : public filter_gate_base {
public:
	using output_type = filter_output<Dim, Elem>;
	using input_type = filter_input<Dim, Elem>;

private:
	output_type& origin_output_;
	input_type& destination_input_;

public:
	filter_gate(output_type& orig_out, input_type& dest_in);
	filter_gate(const filter_gate&) = delete;
	filter_gate& operator=(const filter_gate&) = delete;
	
	output_type& origin_output() const { return origin_output_; }
	filter& origin_filter() const { return origin_output_.this_filter(); }
	
	input_type& destination_input() const { return destination_input_; }
	filter& destination_filter() const { return destination_input_.this_filter(); }
};

}}

#endif
