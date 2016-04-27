#ifndef MF_FLOW_THIN_NODE_H_
#define MF_FLOW_THIN_NODE_H_

#include <thread>
#include <atomic>
#include "node_base.h"
#include "../queue/ndarray_ring.h"
#include "../ndarray/ndarray_view_cast.h"
#include "../utility/event.h"

namespace mf { namespace flow {

template<std::size_t Dim, typename In_elem, typename Out_elem>
class thin_node : public node_base {
public:
	class output;

	using output_type = output;
	using input_type = node_base::input<Dim, In_elem>;

	using output_frame_view_type = typename output_type::frame_view_type;
	using input_frame_view_type = typename input_type::frame_view_type;

private:	
	output_frame_view_type begin_pull_frame_(time_unit t) {
		MF_ASSERT(inputs().size() == 1);
		MF_ASSERT(outputs().size() == 1);
		
		// TODO refactor
		output_type& out = dynamic_cast<output_type&>(outputs().front().get());
		input_type& in = dynamic_cast<input_type&>(inputs().front().get());
		
		set_current_time(t);
		
		this->pre_process();
		
		//if(in.is_activated()) {
			in.begin_read_frame(t);
			
			output_frame_view_type out_view = ndarray_view_reinterpret_cast<output_frame_view_type>(in.view());
			out.set_view(out_view);
		//}
		
		this->process();
		
		out.unset_view();
		
		return out_view;
	}
	
	void end_pull_frame_(bool& was_last) {
		input_base& in = inputs().front();
		if(in.is_activated()) inputs().front().get().end_read_frame(current_time());
		was_last = in.reached_end(current_time());
	}

public:
	thin_node() = default;
};


template<std::size_t Dim, typename In_elem, typename Out_elem>
class thin_node<Dim, In_elem, Out_elem>::output : public node_base::output<Dim, Out_elem> {
	using base = node_base::output<Dim, Out_elem>;

public:
	using typename base::full_view_type;
	using typename base::frame_view_type;

private:
	frame_view_type overwritten_frame_view_;
	time_unit last_time_ = -1;
	time_unit end_time_ = -1;

public:
	using base::base;

	void setup() override{}

	/// \name Write interface.
	// TODO remove? (interface not same for all node types)
	///@{
	bool begin_write_next_frame(time_unit&) override {return false;}
	void end_write_frame(bool mark_end = false) override {}
	void cancel_write_frame() override {}
	///@}
	
	/// \name Read interface, used by connected input.
	///@{
	bool begin_read_span(time_span span, full_view_type& view) override {
		thin_node& nd = dynamic_cast<thin_node&>(base::node()); // TODO change
		MF_ASSERT(span.duration() == 1);
		time_unit t = span.start_time();
		last_time_ = t;
		frame_view_type out_view = nd.begin_pull_frame_(t);
		
		full_view_type vw(out_view.add_front_axis(), t);
		view.reset(vw);
		return true;
	}
	
	void end_read(bool consume_frame) override  {
		thin_node& nd = dynamic_cast<thin_node&>(base::node());
		bool was_last;
		nd.end_pull_frame_(was_last);
		if(was_last) end_time_ = last_time_+1;
	}
	
	time_unit end_time() const override {
		return end_time_;
	}
	///@}
};


}}


#endif
