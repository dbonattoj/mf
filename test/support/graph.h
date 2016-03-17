#ifndef MF_TEST_GRAPH_H_
#define MF_TEST_GRAPH_H_

#include <vector>
#include "../../src/common.h"
#include "../../src/graph/media_sequential_node.h"
#include "../../src/graph/media_sink_node.h"
#include "../../src/graph/media_node_input.h"
#include "../../src/graph/media_node_output.h"
#include "../../src/ndarray/ndcoord.h"

namespace mf {

class simple_frame_source : public media_sequential_node {
private:
	time_unit last_frame_;
	ndsize<2> frame_shape_;

public:
	media_node_output<2, int> output;

	explicit simple_frame_source(time_unit last_frame, const ndsize<2>& frame_shape) :
		last_frame_(last_frame), frame_shape_(frame_shape), output(*this) { }
	
	void setup_() override;
	void process_() override;
	bool process_reached_end_() override;
};


class expected_sequence_sink : public media_sink_node {
private:
	const std::vector<int> expected_sequence_;
	
	bool got_mismatch_ = false;
	std::size_t counter_ = 0;

public:
	media_node_input<2, int> input;
	
	explicit expected_sequence_sink(const std::vector<int>& seq) :
		expected_sequence_(seq), input(*this) { }
	
	void setup_() override { }
	void process_() override;
	bool got_expected_sequence() const;
};


}

#endif
