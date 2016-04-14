#ifndef MF_FLOW_NODE_H_
#define MF_FLOW_NODE_H_

#include <thread>
#include "../common.h"
#include "node_base.h"

namespace mf { namespace flow {
	
template<std::size_t Dim, typename Elem> class node_input;
template<std::size_t Dim, typename Elem> class node_output;


class node : public node_base {
private:
	std::thread thread_;

	void thread_main_();
	void pull_frame_();

protected:
	void stop() override;
	void launch() override;
			
public:
	template<std::size_t Dim, typename Elem> using input_type = node_input<Dim, Elem>;
	template<std::size_t Dim, typename Elem> using output_type = node_output<Dim, Elem>;

	explicit node(time_unit prefetch = 0);
	~node();
};

}}

#endif
