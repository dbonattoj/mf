#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include <vector>
#include <memory>
#include <functional>
#include "ndcoord.h"
#include "ndarray_ring.h"

#include <iostream>

namespace mf {

namespace detail {
	class media_node_input_base {
	public:
		virtual void pull() = 0;
		virtual void did_pull() = 0;
	};
	
	class media_node_base {
	protected:		
		std::vector<media_node_input_base*> inputs_;

		virtual void process_frame_() = 0;
		
		void pull_input_and_process_() {
			for(auto& input : inputs_) input->pull();
			this->process_frame_();
			for(auto& input : inputs_) input->did_pull();
		}
	};
}


template<std::size_t Dim, typename T> class media_node;

template<std::size_t Dim, typename T>
class media_node_input : public detail::media_node_input_base {
public:
	using connected_node_type = media_node<Dim, T>;

private:
	connected_node_type* connected_node_ = nullptr;

public:
	using input_view_type = typename connected_node_type::output_view_type;
	
	input_view_type input_view;
			
	void pull() override {
		input_view.reset(connected_node_->pull());
	}
	
	void did_pull() override {
		connected_node_->did_pull();
	}
	
	void connect(connected_node_type& nd) {
		connected_node_ = &nd;
	}
};


template<std::size_t Dim, typename T>
class media_node : public detail::media_node_base {
protected:
	ndarray_ring<Dim, T> output_;

public:
	using output_view_type = ndarray_view<Dim + 1, const T>;

	explicit media_node(const ndsize<Dim>& frame_shape) :
		output_(frame_shape, 1) { }

	void add_input_(detail::media_node_input_base& input) {
		inputs_.push_back(&input);
	}
		
	output_view_type pull() {
		std::cout << "pulling "<< this << std::endl;
		pull_input_and_process_();
		return output_.read(1);
	}
	
	void did_pull() {
		std::cout << "did pull "<< this << std::endl;

		output_.did_read(1);
	}
};


class media_sink : public detail::media_node_base {
public:
	void add_input_(detail::media_node_input_base& input) {
		inputs_.push_back(&input);
	}
	void pull() {
		pull_input_and_process_();
	}
};

}


#endif
