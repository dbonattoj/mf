#include <catch.hpp>
#include "../support/flow.h"
#include "../support/ndarray.h"
#include <mf/flow/graph.h>
#include <mf/flow/sync_node.h>
#include <mf/flow/async_node.h>
#include <mf/flow/multiplex_node.h>

using namespace mf;
using namespace mf::test;

#if 1
TEST_CASE("flow multiplex", "[.][flow_multiplex]") {
	std::vector<int> seq(150);
	for(int i = 0; i < seq.size(); ++i) seq[i] = i;

	const std::vector<bool>& act1     { 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1,  1,  1 };
	const std::vector<bool>& act2     { 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1,  0,  0 };

	auto shp = make_ndsize(1, 256);

	flow::graph gr;
	auto& source = gr.add_filter<sequence_frame_source>(seq.size()-1, shp, false);
	
	auto& mplx_node = gr.add_node_<flow::multiplex_node>();
	auto& mout1 = mplx_node.add_output();
	auto& mout2 = mplx_node.add_output();
	mplx_node.input().connect(source.output.this_node_output());
	
	auto& filt1 = gr.add_filter<passthrough_filter, flow::sync_node>(0, 0);
	auto& filt2 = gr.add_filter<passthrough_filter, flow::sync_node>(0, 0);
	filt1.input.connect(mout1);
	filt2.input.connect(mout2);
	
	auto& merge = gr.add_filter<input_synchronize_test_filter>();
	merge.input1.connect(filt1.output);
	merge.input2.connect(filt2.output);
	
	auto& sink = gr.add_sink_filter<expected_frames_sink>(seq);
	sink.input.connect(merge.output);

	MF_DEBUG("setup...");
	gr.setup();
	MF_DEBUG("run...");
	gr.callback_function = [&](time_unit t) {
		std::cout << "..." << t << std::endl;
	};
	gr.run();
}
#endif
