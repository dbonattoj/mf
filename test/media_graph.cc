#include <catch.hpp>
#include "../src/graph/media_graph.h"
#include "../src/graph/media_sequential_node.h"
#include "support/graph.h"

using namespace mf;

TEST_CASE("media graph", "[media_graph]") {
	media_graph graph;
	auto shp = make_ndsize(320, 240);
	
	SECTION("only source and sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source = graph.add_node<simple_frame_source>(10, shp);
		auto& sink = graph.add_sink<expected_sequence_sink>(seq);
		sink.input.connect(source.output);
		graph.setup();
		graph.run();
		REQUIRE(sink.got_expected_sequence());
	}
}
