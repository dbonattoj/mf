#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include <vector>
#include <cstdint>
#include "common.h"
#include "ndarray_shared_ring.h"

namespace mf {

class media_node;

namespace detail {
	class media_node_input_base {
	protected:
		time_unit past_window_ = 0;
		time_unit future_window_ = 0;
		
		time_span requested_span_(time_unit t) const {
			if(t < past_window_) {
				// at beginning, not all frames for past window available yet
				return time_span(0, t + future_window_ + 1);
			} else {
				return time_span(t - past_window_, t + future_window_ + 1);
			}
		}
			
	public:
		media_node_input_base(time_unit past_window, time_unit future_window) :
			past_window_(past_window), future_window_(future_window) { }
	
		virtual void pull(time_unit) = 0;
		virtual void begin_read(time_unit) = 0;
		virtual void end_read(time_unit) = 0;
	};
	
	
	class media_node_output_base {	
	protected:
		media_node& node_;

	public:
		explicit media_node_output_base(media_node& nd) : node_(nd) { }
		
		virtual void begin_write() = 0;
		virtual void end_write() = 0;
	};
}


template<std::size_t Dim, typename T>
class media_node_output : public detail::media_node_output_base {
public:
	using read_view_type = ndarray_view<Dim + 1, T>;
	using write_view_type = ndarray_view<Dim, T>;

private:
	ndarray_shared_ring<Dim, T> buffer_;
	write_view_type view_;

public:
	explicit media_node_output(media_node& node, const ndsize<Dim>& frame_shape, time_unit duration = 20) :
		media_node_output_base(node), buffer_(frame_shape, duration) { }
	
	void pull(time_span span);
		
	read_view_type begin_read(time_span span);
	void end_read(time_unit);
	
	virtual void begin_write() override;
	virtual void end_write() override;
	
	write_view_type& view() {
		return view_;
	}
};


template<std::size_t Dim, typename T>
class media_node_input : public detail::media_node_input_base {
public:
	using output_type = media_node_output<Dim, T>;
	using read_view_type = ndarray_view<Dim + 1, T>;
	
private:
	output_type* connected_output_ = nullptr;
	read_view_type view_;

public:
	media_node_input(time_unit past_window = 0, time_unit future_window = 0) :
		media_node_input_base(past_window, future_window) { }
	
	void connect(output_type& output)  {
		connected_output_ = &output;
	}

	void pull(time_unit t) override;
	
	void begin_read(time_unit t) override;
	void end_read(time_unit t) override;
		
	read_view_type& view() {
		return view_;
	}
};


class media_node {
protected:
	std::vector<detail::media_node_input_base*> inputs_;
	std::vector<detail::media_node_output_base*> outputs_;
	time_unit time_ = -1;
	
	void register_input_(detail::media_node_input_base& input) {
		inputs_.push_back(&input);
	}
	
	void register_output_(detail::media_node_output_base& output) {
		outputs_.push_back(&output);
	}
	
	// implemented in concrete subclass
	virtual void process_() = 0;

public:
	virtual void pull_frame() = 0;
	
	time_unit current_time() const noexcept { return time_; }
};


// processes frames only when outputs are pulled
class media_node_sequential : public media_node {
public:
	void pull_frame() override {
		time_++;
		for(auto* input : inputs_) {
			input->pull(time_);
			input->begin_read(time_);
		}
		for(auto* output : outputs_) {
			output->begin_write();
		}
		this->process_();
		for(auto* input : inputs_) {
			input->end_read(time_);
		}
		for(auto* output : outputs_) {
			output->end_write();
		}
	}	
};


class media_node_sink : public media_node_sequential { };


class media_node_source : public media_node_sequential { };


/*
// processes frames continuously in separate thread
class media_node_parallel : public media_node {
public:
	void pull_frame(time_unit t) {
		
	}
};
*/


}

#include "media_node.tcc"

#endif