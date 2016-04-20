#ifndef MF_SYNC_NODE_H_
#define MF_SYNC_NODE_H_

#include "node_base.h"

namespace mf { namespace flow {

namespace detail {
	template<std::size_t Dim, typename Elem> class sync_node_input;
}


template<std::size_t Output_dim, typename Output_elem>
class sync_node : public node_base {
	template<std::size_t Dim, typename Elem> friend class sync_node_input<>;
	
public:
	class output;
	template<std::size_t Dim, typename Elem> using input = sync_node_input<Dim, Elem>;
};


template<std::size_t Output_dim, typename Output_elem>
class sync_node<Output_dim, Output_elem>::output : public node_base::output<Output_dim, Output_elem> {
	using base = node_base::output<Output_dim, Output_elem>;


};


template<std::size_t Dim, typename Elem>
class detail::sync_node_input : public node_base::input<Dim, Elem> {
	using base = node_base::input<Dim, Elem>;
}


}}

#include "sync_node.tcc"

#endif
