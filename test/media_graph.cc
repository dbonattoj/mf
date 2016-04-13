#include <catch.hpp>
#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "support/graph.h"
#include "support/ndarray.h"
#include <iostream>

using namespace mf;
using namespace mf::test;

TEST_CASE("media graph", "[media_graph]") {
	media_graph graph;
	auto shp = make_ndsize(320, 240);

	std::vector<int> seq(100);
	for(int i = 0; i < seq.size(); ++i) seq[i] = i;	

/*
	SECTION("source -> sink") {
		auto& source = graph.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		sink.input.connect(source.output);
		graph.setup();
		graph.run();
		REQUIRE(sink.got_expected_frames());
	}
*/

	SECTION("source --> passthrough --> sink") {
		auto& source = graph.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough = graph.add_node<passthrough_node>(0, 0);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		passthrough.input.connect(source.output);
		sink.input.connect(passthrough.output);
		graph.setup();
		graph.run();
		REQUIRE(sink.got_expected_frames());
	}

}
