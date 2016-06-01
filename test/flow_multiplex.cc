#include <catch.hpp>
#include "support/flow.h"
#include "support/ndarray.h"
#include <mf/flow/graph.h>
#include <mf/flow/sync_node.h>
#include <mf/flow/multiplex_node.h>
#include <iostream>

using namespace mf;
using namespace mf::test;

TEST_CASE("flow multiplex", "[flow_multiplex]") {
	std::vector<int> seq(5);
	for(int i = 0; i < seq.size(); ++i) seq[i] = i;

	flow::graph gr;
	auto& mplx_node = gr.add_node_<flow::multiplex_node>();
	auto& mout1 = mplx_node.add_output(frame_format::default_format<int>());
	auto& mout2 = mplx_node.add_output(frame_format::default_format<int>());
	
	auto& filt1 = gr.add_filter<passthrough_filter>(0, 0);
	auto& filt2 = gr.add_filter<passthrough_filter>(0, 0);
	filt1.input.connect(mout1);
	filt2.input.connect(mout2);
	
	auto& merge = gr.add_filter<input_synchronize_test_filter>();
	merge.input1.connect(filt1.output);
	merge.input2.connect(filt2.output);

	auto& sink = gr.add_sink_filter<expected_frames_sink>(seq);
	sink.input.connect(merge.output);
	
	std::cout << "setup..." << std::endl;
	gr.setup();
	std::cout << "setup." << std::endl;
	gr.run();
}
