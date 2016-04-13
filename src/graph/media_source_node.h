#ifndef MF_MEDIA_SOURCE_NODE_H_
#define MF_MEDIA_SOURCE_NODE_H_

#include <thread>
#include "../common.h"
#include "media_node_base.h"

namespace mf {
	
template<std::size_t Dim, typename Elem> class media_node_output;

class media_source_node : public media_node_base {
private:
	std::thread thread_;

	void thread_main_();
	void pull_frame_();

protected:
	void stop_() override;
	void launch_() override;
	
	virtual bool reached_end() const { throw std::runtime_error("not implemented"); } // called for forward source only
			
public:
	template<std::size_t Dim, typename Elem> using output_type = media_node_output<Dim, Elem>;

	media_source_node(); // forward source

	explicit media_source_node(bool seekable, time_unit stream_duration = -1);
	~media_source_node();
};

}

#endif
