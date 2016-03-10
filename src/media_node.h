#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include <vector>
#include <cstdint>
#include "ndarray_shared_ring.h"

namespace mf {


using time_unit = std::int32_t;


class media_node;


namespace detail {
	class media_node_input_base {
	protected:
		time_unit past_window_;
		time_unit future_window_;	
	public:
		virtual void begin_read(time_unit) = 0;
		virtual void end_read() = 0;
	};
	
	
	class media_node_output_base {	
	protected:
		media_node& node_;

	public:
		explicit media_node_output_base(media_node& nd) : node_(nd) { }
	};
}


template<std::size_t Dim, typename T>
class media_node_output : public media_node_output_base {
private:
	ndarray_shared_ring<Dim, T> buffer_;
};


template<std::size_t Dim, typename T>
class media_node_input : public media_node_input_base {
public:
	using output_type = media_node_output<Dim, T>;
	using read_view_type = ndarray_view<Dim, T>;
	
private:
	output_type* connected_output_ = nullptr;
	read_view_type view_;

public:
	void begin_read(time_unit) override {
		
	}
	
	void end_read() override {
		
	}
	
	read_view_type& view();
};


class media_node {
private:
	std::vector<media_node_input_base*> inputs_;
	std::vector<media_node_output_base*> outputs_;
	time_unit time_;
	
protected:
	void register_input_(media_node_input_base& input) {
		inputs_.push_back(&input);
	}
	
	void register_output_(media_node_output_base& output) {
		outputs_.push_back(&output);
	}
	
	// implemented in concrete subclass
	virtual void process_() = 0;

public:
	virtual void pull_frame(time_unit t) = 0;
};


// processes frames only when outputs are pulled
class media_node_sequential : public media_node {
public:
	void pull_frame(time_unit t) override {
		for(media_node_input_base* input : inputs_) {
			input_->pull(t);
			input_->begin_read(t);
		}
		for(media_node_output_base* output : outputs_) {
			output_->begin_write();
		}
		this->process_();
		for(media_node_input_base* input : inputs_) {
			input_->end_read(t);
		}
		for(media_node_output_base* output : outputs_) {
			output_->end_write();
		}
	}	
};


class media_sink : public media_node_sequential {

};


// processes frames continuously in separate thread
class media_node_parallel : public media_node {
public:
	void pull_frame(time_unit t) override {
		
	}
};



}

#endif