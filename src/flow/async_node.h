#ifndef MF_FLOW_ASYNC_NODE_H_
#define MF_FLOW_ASYNC_NODE_H_

#include "filter_node.h"
#include "../queue/shared_ring.h"
#include <thread>

namespace mf { namespace flow {

class graph;
class async_node;


class async_node_output : public node_output {
private:
	std::unique_ptr<shared_ring> ring_;

public:
	using node_type = async_node;

	using node_output::node_output;

	void setup() override;

	/// \name Read interface, used by connected input.
	///@{
	void pull(time_span span) override;
	timed_frames_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
	time_unit end_time() const override;
	///@}
	
	/// \name Write interface, used by node.
	///@{
	frame_view begin_write_frame(time_unit& t) override;
	void end_write_frame(bool was_last_frame) override;
	void cancel_write_frame() override;
	///@}
};



/// Asynchronous node base class.
/** Processes frames in a separate thread owned by the node. Can have multiple inputs, but only one output. Can process
 ** frames `t+k` (`k <= 1`), at the same time that current frame `t` is being read or processed by suceeding nodes
 ** in graph. */
class async_node final : public filter_node {
private:
	bool running_ = false;
	std::thread thread_;
	
	void thread_main_();

public:
	explicit async_node(graph&);
	~async_node();
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
	bool process_next_frame() override;
	
	node_input& add_input(time_unit past_window, time_unit future_window) override {
		return add_input_<node_input>(past_window, future_window);
	}
	
	async_node_output& add_output(const frame_format& format) override {
		return add_output_<async_node_output>(format);
	}
};

}}

#endif
