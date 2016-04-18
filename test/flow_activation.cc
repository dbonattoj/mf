#include <catch.hpp>
#include "../src/flow/graph.h"
#include "../src/flow/node.h"
#include "support/flow.h"
#include "support/ndarray.h"
#include <iostream>

using namespace mf;
using namespace mf::test;

TEST_CASE("media_graph_seekable", "[flow_graph][activation]") {
	//set_debug_mode(debug_mode::cerr);
	
	flow::graph gr;
	auto shp = make_ndsize(320, 240);
	
	SECTION("input activation") {
		const std::vector<int>& seq { 0, 1, noframe, noframe, noframe, noframe, 6, 7, noframe, noframe, 10 };
		const std::vector<bool>& act  { 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1 };

		const std::vector<bool>& act1 { 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1 };
		const std::vector<bool>& act2 { 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1 };

		SECTION("source -> sink") {
			auto& source = gr.add_node<sequence_frame_source>(10, shp, true);
			auto& sink = gr.add_sink<expected_frames_sink>(seq);
			sink.input.connect(source.output);
			sink.activation = act;
			gr.setup();
			gr.run();
			REQUIRE(sink.check());
		}
	
		SECTION("source -> passthrough -> sink") {
			auto& source = gr.add_node<sequence_frame_source>(10, shp, true);
			auto& passthrough = gr.add_node<passthrough_node>(0, 0);
			auto& sink = gr.add_sink<expected_frames_sink>(seq);
			passthrough.input.connect(source.output);
			sink.input.connect(passthrough.output);
			passthrough.activation = act1;
			sink.activation = act2;
			gr.setup();
			gr.launch();
			gr.run();
			REQUIRE(sink.check());
		}
		
	}
	
}
