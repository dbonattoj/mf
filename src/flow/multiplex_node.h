#ifndef MF_FLOW_MULTIPLEX_NODE_H_
#define MF_FLOW_MULTIPLEX_NODE_H_

#include "node.h"
#include <thread>
#include <condition_variable>

namespace mf { namespace flow {

class graph;


class multiplex_node final : public node {
private:
	const node* successor_node_ = nullptr;
	
	std::thread thread_;
	std::atomic<time_unit> pull_request_time_ = -1;
	std::condition_variable request_time_change_;
	
	void thread_main_();
	
public:
	explicit multiplex_node(graph&);
	~multiplex_node() override;

	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
	
	void launch() override;
	void stop() override;
	void pre_setup() override;
	
	node_input& input();
	multiplex_node_output& add_output();
};


class multiplex_node_output final : public node_output {
	multiplex_node& this_node() noexcept;
	const multiplex_node& this_node() const noexcept;
	
public:
	using node_output::node_output;
	
	node::pull_result pull(time_span& span, bool reconnect) override;
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


inline multiplex_node& multiplex_node_output::this_node() noexcept {
	return static_cast<multiplex_node&>(node_output::this_node());
}


inline const multiplex_node& multiplex_node_output::this_node() const noexcept {
	return static_cast<const multiplex_node&>(node_output::this_node());
}



}}

#endif

