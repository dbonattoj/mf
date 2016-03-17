#ifndef MF_TEST_GRAPH_H_
#define MF_TEST_GRAPH_H_

#include <vector>
#include <functional>
#include "../../src/common.h"
#include "../../src/graph/media_sequential_node.h"
#include "../../src/graph/media_sink_node.h"
#include "../../src/graph/media_node_input.h"
#include "../../src/graph/media_node_output.h"
#include "../../src/ndarray/ndcoord.h"

namespace mf {

class sequence_frame_source : public media_sequential_node {
private:
	time_unit last_frame_;
	ndsize<2> frame_shape_;

public:
	media_node_output<2, int> output;

	explicit sequence_frame_source(time_unit last_frame, const ndsize<2>& frame_shape) :
		last_frame_(last_frame), frame_shape_(frame_shape), output(*this) { }
	
	void setup_() override;
	void process_() override;
	bool process_reached_end_() override;
};


class expected_frames_sink : public media_sink_node {
private:
	const std::vector<int> expected_frames_;
	
	bool got_mismatch_ = false;
	std::size_t counter_ = 0;

public:
	media_node_input<2, int> input;
	
	explicit expected_frames_sink(const std::vector<int>& seq) :
		expected_frames_(seq), input(*this) { }
	
	void setup_() override { }
	void process_() override;
	
	bool got_expected_frames() const;
};


class callback_node : public media_sequential_node {
public:
	using input_type = media_node_input<2, int>;
	using output_type = media_node_output<2, int>;
	using callback_func = void(callback_node& self, input_type& in, output_type& out);
	
private:
	std::function<callback_func> callback_;
	
	void setup_() override;
	void process_() override;
	
public:
	input_type input;
	output_type output;

	callback_node(time_unit past_window, time_unit future_window);
		
	template<typename Function>
	void set_callback(Function func) {
		callback_ = func;
	}
};


}

#endif
