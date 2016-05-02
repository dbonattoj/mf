#ifndef MF_FLOW_SYNC_NODE_H_
#define MF_FLOW_SYNC_NODE_H_

#include "node.h"

namespace mf { namespace flow {

class sync_node_output;

/// Synchronous node base class.
/** Processes frames synchronously when pulled from output. Can have multiple inputs, but only one output. */
class sync_node : public flow {
private:
	time_unit end_time = -1;

protected:
	virtual void setup() { }
	virtual void pre_process(time_unit t) { }
	virtual void process(job&) = 0;
	virtual bool reached_end(time_unit t) const { return false; }

public:
	explicit sync_node(graph& gr) : node(gr) { }
	
	template<std::size_t Dim, typename Elem>
	using input_type = node_input_wrapper<node_input, Dim, Elem>;
	
	template<std::size_t Dim, typename Elem>
	using output_type = node_output_wrapper<sync_node_output, Dim, Elem>;
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
	
	void pull(time_unit t);
	time_unit end_time() const noexcept { return end_time_; }
};


class sync_node_output : public node_output {
private:
	std::unique_ptr<timed_ring> ring_;

public:
	sync_node_output(sync_node& nd) : node_output(nd) { }
	
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
	///@}
};

}}

#endif
