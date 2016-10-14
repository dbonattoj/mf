#ifndef MF_FLOW_REALTIME_GATE_NODE_H_
#define MF_FLOW_REALTIME_GATE_NODE_H_

#include "gate_node.h"
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace mf { namespace flow {

class realtime_gate_node : public gate_node {
	// connect
	// animated 
	//  to
	// realtime

private:
	using frame_buffer_type = ndarray_opaque<1>;
	
	std::thread thread_;
	std::unique_ptr<frame_buffer_type> back_buffer_;
	std::unique_ptr<frame_buffer_type> front_buffer_;
	std::mutex mutex_;
	
	clock_time_point launch_clock_time_;
		
	void thread_main_();
	void load_new_frame_();
	
	time_unit current_output_time_;

	// constantly reads 1 frame on separate thread
	// any copies it into retained_frame_ (mutex-protected)
	// --> use two retained_frames_, and do buffer swap
	// frame index of new frame to load:
	//   calculated from pull clock times
	//   --> expected next clock time --> frame time to pull

public:	
	explicit realtime_gate_node(node_graph&);

	void launch() override;
	void pre_stop() override;
	void stop() override;
	void pre_setup() override;
	void setup() override;
	
	void output_pre_pull_(const time_span&) override;
	node::pull_result output_pull_(time_span& span) override;
	node_frame_window_view output_begin_read_(time_unit duration) override;
	void output_end_read_(time_unit duration) override;
};


inline bool is_realtime_gate_node(const node& nd) {
	return (dynamic_cast<const realtime_gate_node*>(&nd) != nullptr);
}


}}

#endif
