#ifndef MF_FLOW_MULTIPLEX_NODE_H_
#define MF_FLOW_MULTIPLEX_NODE_H_

#include "node.h"
#include "../queue/shared_ring.h"
#include <thread>
#include <memory>

namespace mf { namespace flow {

class multiplex_node;


class multiplex_node_output : public node_output {
private:
	std::unique_ptr<shared_ring> ring_;

public:
	using node_type = multiplex_node;

	shared_ring& ring() { return *ring_; }

	using node_output::node_output;
	
	void setup() override;
	
	/// \name Read interface, used by connected input.
	///@{
	void pull(time_span) override;
	timed_frame_array_view begin_read(time_unit duration) override;
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



class multiplex_node final : public node {
private:
	using output_rings_vector_type = std::vector<std::reference_wrapper<shared_ring>>;
	
	//node_input& input_;
	std::thread thread_;
	
	output_rings_vector_type output_rings_();
	
	time_span curspan;
	
	void thread_main_();

public:
	explicit multiplex_node(graph& gr) : node(gr) { }
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
		
	bool process_next_frame() override;
	
	multiplex_node_output& add_output(const frame_format& format) {
		return add_output_<multiplex_node_output>(format);
	}
};

}}

#endif
