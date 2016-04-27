#ifndef MF_FLOW_THIN_NODE_H_
#define MF_FLOW_THIN_NODE_H_

#include <thread>
#include <atomic>
#include "node_base.h"
#include "../queue/ndarray_ring.h"
#include "../utility/event.h"
/*
namespace mf { namespace flow {

template<std::size_t Dim, typename In_elem, typename Out_elem>
class thin_node : public node_base {
public:
	class output;

	using output_type = output<Dim, Out_elem>;
	using input_type = node_base::input<Dim, In_elem>;

private:	
	void frame_(time_unit t, const full_view_type& view);

	Output_view begin_pull_frame_(time_unit t) {
		MF_ASSERT(inputs().size() == 1);
		MF_ASSERT(outputs().size() == 1);
		
		input_base& in = inputs().front();
		
		if(in.is_activated()) inputs().front().begin_read_frame(1);
		
		
		
		//ndarray_view_reinterpret_cast
		
	}
	
	void end_pull_frame_() {
		input_base& in = inputs().front();
		if(in.is_activated()) inputs().front().end_read_frame();
	}

public:
	thin_node();
};


template<std::size_t Dim, typename T>
class thin_node::output : public node_base::output<Dim, T> {
	using base = node_base::output<Dim, T>;

public:
	using typename base::full_view_type;
	using typename base::frame_view_type;

private:
	frame_view_type overwritten_frame_view_;

public:
	using base::base;

	void setup() override;

	/// \name Write interface.
	// TODO remove? (interface not same for all node types)
	///@{
	bool begin_write_next_frame(time_unit&) override;
	void end_write_frame(bool mark_end = false) override;
	void cancel_write_frame() override;
	///@}
	
	/// \name Read interface, used by connected input.
	///@{
	bool begin_read_span(time_span span, full_view_type& view) override {
		MF_ASSERT(span.duration() == 1);
		
		time_unit t = span.start_time();
		
		// pull from node, 
		
		full_view_type out_view(overwritten_frame_view_.add_front_axis(), t);
		view.reset(out_view);
	}
	
	void end_read(bool consume_frame) override;
	time_unit end_time() const override;
	///@}
};


}}

#include "thin_node.tcc"
*/
#endif
