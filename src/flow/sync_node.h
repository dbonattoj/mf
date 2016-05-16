#ifndef MF_FLOW_SYNC_NODE_H_
#define MF_FLOW_SYNC_NODE_H_

#include <memory>
#include "filter_node.h"
#include "node_job.h"
#include "node_io_wrapper.h"
#include "node_parameter.h"
#include "../queue/frame.h"
#include "../queue/timed_ring.h"

namespace mf { namespace flow {

class sync_node_output;

/// Synchronous node base class.
/** Processes frames synchronously when pulled from output. Can have multiple inputs, but only one output. */
class sync_node final : public filter_node {
public:
	explicit sync_node(graph& gr) : filter_node(gr) { }
	
	template<std::size_t Dim, typename Elem>
	using input_type = node_input_wrapper<node_input, Dim, Elem>;
	
	template<std::size_t Dim, typename Elem>
	using output_type = node_output_wrapper<sync_node_output, Dim, Elem>;
	
	template<typename Value>
	using parameter_type = node_parameter<Value>;
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
		
	bool process_next_frame() override;

	node_input& add_input(time_unit past_window, time_unit future_window) override {
		return add_input<node_input>(past_window, future_window);
	}
	
	sync_node_output& add_output(const frame_format& format) override {
		return add_output<sync_node_output>(format);
	}
};



class sync_node_output : public node_output {
private:
	std::unique_ptr<timed_ring> ring_;

public:
	using node_type = sync_node;

	using node_output::node_output;
	
	void setup() override;
	
	/// \name Read interface, used by connected input.
	///@{
	void pull(time_span) override;
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
