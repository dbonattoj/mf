#ifndef MF_MEDIA_SOURCE_NODE_H_
#define MF_MEDIA_SOURCE_NODE_H_

#include <thread>
#include "../common.h"
#include "node_base.h"

namespace mf { namespace flow {
	
template<std::size_t Dim, typename Elem> class node_output;

class source_node : public node_base {
private:
	std::thread thread_;

	void thread_main_();
	void pull_frame_();

protected:
	void stop_() override;
	void launch_() override;
	
	virtual bool reached_end() const { throw std::runtime_error("not implemented"); } // called for forward source only
			
public:
	template<std::size_t Dim, typename Elem> using output_type = node_output<Dim, Elem>;

	source_node(); // forward source

	explicit source_node(bool seekable, time_unit stream_duration = -1);
	~source_node();
};

}}

#endif
