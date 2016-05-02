#ifndef MF_FLOW_ASYNC_NODE_H_
#define MF_FLOW_ASYNC_NODE_H_

#include "node.h"

namespace mf { namespace flow {

class async_node : public node {
protected:
	bool running_ = false;
	std::thread thread_;
	event stop_event_;
	
	bool frame_();
	void thread_main_();
	
	virtual void setup() { }
	virtual void pre_process(time_unit t) { }
	virtual void process(job&) = 0;

public:
	async_node();
	~async_node();
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
	void pull(time_unit t) final override;
};


class async_node_output : public node_output {
private:
	std::size_t required_ring_capacity_ = -1;
	std::unique_ptr<shared_ring> ring_;

public:	
	void setup() override;

	/// \name Read interface, used by connected input.
	///@{
	void pull(time_unit t) override;
	timed_frames_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
	time_unit end_time() const override;
	///@}
	
	/// \name Write interface, used by node.
	///@{
	frame_view begin_write_frame(time_unit& t) override;
	void end_write_frame(bool was_last_frame) override;
	///@}
};

}}

#endif