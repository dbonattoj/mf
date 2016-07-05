#ifndef MF_FLOW_MULTIPLEX_NODE_H_
#define MF_FLOW_MULTIPLEX_NODE_H_

#include "node.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>

namespace mf { namespace flow {

class graph;
class multiplex_node;

class multiplex_node_output final : public node_output {
private:
	std::shared_lock<std::shared_timed_mutex> input_view_shared_lock_;
	
	multiplex_node& this_node() noexcept;
	const multiplex_node& this_node() const noexcept;
	
public:
	multiplex_node_output(node& nd, std::ptrdiff_t index);
	
	node::pull_result pull(time_span& span, bool reconnect) override;
	timed_frame_array_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


class multiplex_node final : public node {
	friend class multiplex_node_output;

private:
	const node* successor_node_ = nullptr;
	time_unit input_past_window_ = -1;
	time_unit input_future_window_ = -1;

	std::thread thread_;

	bool stopped_ = false;
	time_unit successor_time_of_input_view_ = -1;
	timed_frame_array_view input_view_;
	
	std::mutex successor_time_mutex_;
	std::condition_variable successor_time_changed_cv_;

	std::shared_timed_mutex input_view_mutex_;
	std::condition_variable_any input_view_updated_cv_;
	
	time_span expected_input_span_() const;
	void load_input_view_(time_unit t);
	void thread_main_();
	
public:
	explicit multiplex_node(graph&);
	~multiplex_node() override;

	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
	
	void launch() override;
	void stop() override;
	void pre_setup() override;
	void setup() override;
	
	node_input& input();
	multiplex_node_output& add_output();
};


inline multiplex_node& multiplex_node_output::this_node() noexcept {
	return static_cast<multiplex_node&>(node_output::this_node());
}

inline const multiplex_node& multiplex_node_output::this_node() const noexcept {
	return static_cast<const multiplex_node&>(node_output::this_node());
}



}}

#endif

