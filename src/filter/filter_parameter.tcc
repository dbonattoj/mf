#include "filter.h"
#include "filter_graph.h"

namespace mf { namespace flow {

template<typename Value>
filter_parameter<Value>::filter_parameter(filter& filt) :
	filter_(filt),
	id_(filter_.this_graph().new_parameter_id()) { }


template<typename Value>
filter_extern_parameter<Value>::filter_extern_parameter(filter& filt) :
	filter_(filt),
	linked_id_(undefined_parameter_id) { }
	

template<typename Value>
void filter_extern_parameter<Value>::link(parameter_type& param) {
	linked_id_ = param.id();
}


template<typename Value>
bool filter_extern_parameter<Value>::is_linked() const {
	return (linked_id_ != undefined_parameter_id);
}


}}
