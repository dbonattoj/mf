#ifndef MF_FLOW_THIN_NODE_H_
#define MF_FLOW_THIN_NODE_H_

#include "node.h"
#include "node_io_wrapper.h"

namespace mf { namespace flow {

/// Thin node base class.
/** Has one input and one output, and input has no time window. Allocates no buffer for output, but concrete subclass
 ** must instead do element-wise in-place processing of data. Frame format (i.e. element stride and alignment) of input
 ** and output are adjusted to enable this. */
class thin_node : public node {
private:
	node_input& input_() const { return inputs().front().get(); }
	node_output& output_() const { return output().front().get(); }

protected:
	virtual void setup() { }
	virtual void process(job&) = 0;

public:
	template<std::size_t Dim, typename Elem>
	using input_type = node_input_wrapper<node_input, Dim, Elem>;
	
	template<std::size_t Dim, typename Elem>
	using output_type = node_output_wrapper<thin_node_output, Dim, Elem>;

	explicit thin_node(graph&);
	~thin_node();
	
	void internal_setup() final override;
	void launch() final override;
	void stop() final override;
};


class thin_node_output : public node_output {
private:
	timed_frame_view overwritten_frames_;

public:
	thin_node_output(thin_node& nd) : node_output(nd) { }

	void setup() override;

	/// \name Read interface, used by connected input.
	///@{
	void pull(time_span t) override;
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
