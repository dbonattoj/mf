#ifndef MF_FLOW_MULTIPLEX_NODE_LOADER_H_
#define MF_FLOW_MULTIPLEX_NODE_LOADER_H_

#include "multiplex_node.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <atomic>

namespace mf { namespace flow {

class multiplex_node::loader {
private:
	multiplex_node& node_;
	thread_index thread_index_ = undefined_thread_index;

protected:
	multiplex_node& this_node() { return node_; }
	const multiplex_node& this_node() const { return node_; }

public:
	loader(multiplex_node&, thread_index);
	virtual ~loader() = default;
	
	thread_index loader_thread_index() const { return thread_index_; }
	virtual bool is_async() const = 0;
	
	virtual void stop() = 0;
	virtual void launch() = 0;
	virtual node::pull_result pull(time_span span) = 0;
	virtual timed_frame_array_view begin_read(time_span span) = 0;
	virtual void end_read(time_unit duration) = 0;
};


///////////////


class multiplex_node::sync_loader : public multiplex_node::loader {
public:
	explicit sync_loader(multiplex_node&);

	bool is_async() const override { return false; }

	void stop() override;
	void launch() override;
	node::pull_result pull(time_span span) override;
	timed_frame_array_view begin_read(time_span span) override;
	void end_read(time_unit duration) override;
};


///////////////


class multiplex_node::async_loader : public multiplex_node::loader {
private:
	std::thread thread_;
	std::atomic<bool> stopped_ {false};
	
	std::mutex successor_time_mutex_;	
	std::condition_variable successor_time_changed_cv_;

	std::shared_timed_mutex input_view_mutex_;
	std::condition_variable_any input_view_updated_cv_;

	void thread_main_();

public:
	explicit async_loader(multiplex_node&);
	~async_loader() override;

	bool is_async() const override { return true; }

	void stop() override;
	void launch() override;
	node::pull_result pull(time_span span) override;
	timed_frame_array_view begin_read(time_span span) override;
	void end_read(time_unit duration) override;
};

}}

#endif
