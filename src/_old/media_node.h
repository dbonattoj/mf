#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include <vector>
#include <condition_variable>
#include "ndarray_ring.h"

namespace mf {


namespace detail {
	class media_node_output_base {
	public:
		virtual void begin_write();
		virtual void end_write();
	};

	class media_node_input_base {
	public:
		virtual void begin_read(int t);
		virtual void end_read();
	};
}


struct time_range {
	int from;
	int to;
};


class media_node;


template<std::size_t Dim, typename T>
class media_node_output : public media_node_output_base {
public:
	using write_view_type = ndarray_view<Dim, T>;
	using read_view_type = ndarray_view<Dim + 1, T>;

private:
	media_node& node_;
	ndarray_ring<Dim, T> buffer_;
	int available_start_time_;
	int available_end_time_;
	
	write_view_type write_view_;

public:
	void begin_write() override;
	void end_write() override;

	bool view_available() const;
	write_view_type& view();
	
	
	read_view_type pull(time_range rng);
	void did_pull();
};


template<std::size_t Dim, typename T>
class media_node_input : public media_node_input_base {
public:
	using read_view_type = ndarray_view<Dim + 1, T>;
	
private:
	media_node_output<Dim, T>* connected_output_ = nullptr;
	int past_window_;
	int future_window_;
	
	read_view_type read_view_;

public:
	void begin_read(int t) override;
	void end_read() override;
	
	bool view_available() const;
	read_view_type& view();
};



class media_node {
private:
	std::vector<media_node_output_base*> outputs_;
	std::vector<media_node_input_base*> inputs_;
	int time_;
	
	void pull_inputs_();
	void release_inputs_();
	void push_outputs_();
	void release_outputs_();


public:
	

protected:
	virtual void process_() = 0;

	void register_output_(media_node_output_base& out) {
		outputs_.push_back(&out);
	}

	void register_input_(media_node_input_base& in) {
		outputs_.push_back(&in);
	}
	
	int time() const { return time_; }
	
	virtual void pull_frame(int t);
};


class asynchronous_media_node : public media_node {
private:
	std::condition_variable time_step_cv_;
	std::thread thread_;
	
	void thread_main_();
	
public:
	void pull_frame(int t) override;
};

#endif
