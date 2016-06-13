#include <catch.hpp>
#include "../support/flow.h"
#include "../support/ndarray.h"
#include <mf/flow/graph.h>
#include <mf/flow/sync_node.h>
#include <mf/flow/async_node.h>
#include <mf/flow/multiplex_node.h>

using namespace mf;
using namespace mf::test;

TEST_CASE("flow multiplex", "[.][flow_multiplex]") {
	std::vector<int> seq(150);
	for(int i = 0; i < seq.size(); ++i) seq[i] = i;

	auto shp = make_ndsize(1, 256);

	flow::graph gr;
	auto& source = gr.add_filter<sequence_frame_source>(seq.size()-1, shp, false);
	auto& mplx_node = gr.add_node_<flow::multiplex_node>();
	mplx_node.input().connect(source.output.this_node_output());
	auto& mout1 = mplx_node.add_output(frame_format::default_format<int>());
	auto& mout2 = mplx_node.add_output(frame_format::default_format<int>());
	auto& mout3 = mplx_node.add_output(frame_format::default_format<int>());

	const std::vector<bool>& act1     { 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1,  1,  1 };
	const std::vector<bool>& act2     { 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1,  0,  0 };

	
	auto& filt1 = gr.add_filter<passthrough_filter, flow::async_node>(3, 4);
	auto& filt2 = gr.add_filter<passthrough_filter, flow::async_node>(5, 2);
	auto& filt3 = gr.add_filter<passthrough_filter, flow::async_node>(1, 1);
	filt1.this_node().set_prefetch_duration(4);
//	filt2.this_node().set_prefetch_duration(0);
	filt3.this_node().set_prefetch_duration(5);
	filt1.activation = act1;
	filt1.input.connect(mout1);
	filt2.activation = act1;
	filt2.input.connect(mout2);
	filt3.input.connect(filt2.output);
	
	auto& merge = gr.add_filter<input_synchronize_test_filter>();
	merge.input1.connect(filt1.output);
	merge.input2.connect(filt3.output);

	auto& merge2 = gr.add_filter<input_synchronize_test_filter>();
	merge2.input1.connect(merge.output);
	merge2.input2.connect(mout3);

	auto& sink = gr.add_sink_filter<expected_frames_sink>(seq);
	sink.input.connect(merge2.output);

	MF_DEBUG("setup...");
	gr.setup();
	MF_DEBUG("run...");
	gr.callback_function = [&](time_unit t) {
		std::cout << "..." << t << std::endl;
	};
	gr.run();
}
