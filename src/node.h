#ifndef MF_NODE_H_
#define MF_NODE_H_

#include "ndarray_ring.h"
#include <vector>

namespace mf {

namespace detail {
	class node_base {
	public:
		virtual void pull() = 0;
	};
}


template<std::size_t Dim, typename T>
class node : public detail::node_base {
protected:
	int time_;
	ndarray_ring<Dim, T> output_;
	std::vector<node_base*> input_nodes_;
	std::size_t time_window_;
	
	node(ndsize<Dim> shape, std::size_t time_window);

	virtual void process_frame_(const ndarray_view<Dim, T>&) = 0;

public:
	void pull() override;
	
	void connect_input(node*);
};

}

#include "node.tcc"

#endif
