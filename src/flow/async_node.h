#ifndef MF_FLOW_ASYNC_NODE_H_
#define MF_FLOW_ASYNC_NODE_H_

#include "node.h"
#include "node_job.h"
#include "node_io_wrapper.h"
#include "node_parameter.h"
#include "../queue/shared_ring.h"
#include <thread>

namespace mf { namespace flow {

class graph;
class async_node_output;


/// Asynchronous node base class.
/** Processes frames in a separate thread owned by the node. Can have multiple inputs, but only one output. Can process
 ** frames `t+k` (`k <= 1`), at the same time that current frame `t` is being read or processed by suceeding nodes
 ** in graph. */
class async_node : public node {
private:
	bool running_ = false;
	std::thread thread_;
	
	void thread_main_();

protected:
	virtual void setup() { }
	virtual void pre_process(node_job&) { }
	virtual void process(node_job&) = 0;

public:
	template<std::size_t Dim, typename Elem>
	using input_type = node_input_wrapper<node_input, Dim, Elem>;
	
	template<std::size_t Dim, typename Elem>
	using output_type = node_output_wrapper<async_node_output, Dim, Elem>;

	template<typename Value>
	using parameter_type = node_parameter<Value>;

	explicit async_node(graph&);
	~async_node();
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
	bool process_next_frame() override;
};


/// Asynchronous source node base class.
class async_source_node : public async_node {
public:
	explicit async_source_node(graph& gr, bool seekable = false, time_unit stream_duration = -1) :
		async_node(gr)
	{
		define_source_stream_properties(seekable, stream_duration);
	}
};


class async_node_output : public node_output {
private:
	std::unique_ptr<shared_ring> ring_;

public:
	using node_type = async_node;

	async_node_output(async_node& nd, const frame_format& format) :
		node_output(nd, format) { }

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


}}

#endif
