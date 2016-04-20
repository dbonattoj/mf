#ifndef MF_FLOW_ASYNC_NODE_H_
#define MF_FLOW_ASYNC_NODE_H_

#include <thread>
#include <atomic>
#include "node_base.h"
#include "../ndarray/ndarray_shared_ring.h"

namespace mf { namespace flow {

// TODO better doc
/// Asynchronous node base class.
/** Processes frames in a separate thread owned by the node. Can have multiple inputs and outputs. Can process
 ** frames $t+k (k \geq 1)$, at the same time that current frame $t$ is being read or processed by suceeding nodes. */
class async_node : public node_base {
public:
	template<std::size_t Dim, typename Elem> class output;
	
private:
	bool running_ = false;
	std::thread thread_;
	std::atomic<time_unit> time_limit_{-1};
	
	void thread_main_();
	void frame_();

public:
	template<std::size_t Dim, typename Elem> using output_type = output<Dim, Elem>;
	template<std::size_t Dim, typename Elem> using input_type = node_base::input<Dim, Elem>;

	async_node();
	~async_node();

	void stop() override;
	void launch() override;
};


class async_source_node : public async_node {
public:
	async_source_node(bool seekable, time_unit stream_duration = -1);
	
	bool reached_end() const noexcept override;
};


template<std::size_t Dim, typename T>
class async_node::output : public node_base::output<Dim, T> {
	using base = node_base::output<Dim, T>;

private:
	using ring_type = ndarray_shared_ring<Dim, T>;
	
	std::unique_ptr<ring_type> ring_;

public:
	using base::base;

	using typename base::full_view_type;
	using typename base::frame_view_type;

	void setup() override;

	/// \name Write interface.
	/// Called by \ref async_node for writing to the output.
	///@{
	time_unit begin_write_next_frame() override;
	void end_write_frame(bool mark_end = false) override;
	void cancel_write_frame() override;
	///@}
	
	/// \name Read interface, used by connected input.
	/// Called by connected \ref node_base::input for reading from the output.
	///@{
	full_view_type begin_read_span(time_span) override;
	void end_read(bool consume_frame) override;
	void skip_frame() override;
	time_unit end_time() const override;
	///@}
};


}}

#include "async_node.tcc"

#endif
