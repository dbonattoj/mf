#include <catch.hpp>
#include "../src/graph/media_graph.h"
#include "../src/graph/media_sequential_node.h"
#include "support/graph.h"
#include "support/ndarray.h"

using namespace mf;

TEST_CASE("media graph", "[media_graph]") {
	media_graph graph;
	auto shp = make_ndsize(320, 240);
	
	SECTION("source -> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source = graph.add_node<sequence_frame_source>(10, shp);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		sink.input.connect(source.output);
		graph.setup();
		graph.run();
		REQUIRE(sink.got_expected_frames());
	}
	
	SECTION("source -> node -> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source = graph.add_node<sequence_frame_source>(10, shp);
		auto& passthrough = graph.add_node<callback_node>(0, 0);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		passthrough.input.connect(source.output);
		passthrough.set_callback([](callback_node& self, auto& in, auto& out) {
			out.view() = in.view();
		});
		sink.input.connect(passthrough.output);
		graph.setup();
		graph.run();
		REQUIRE(sink.got_expected_frames());
	}
	
	SECTION("past window") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5 };
		auto& source = graph.add_node<sequence_frame_source>(5, shp);
		auto& node = graph.add_node<callback_node>(3, 0);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		node.input.connect(source.output);
		sink.input.connect(node.output);
		graph.setup();
		
		node.set_callback([&](callback_node& self, auto& in, auto& out) {
			REQUIRE(self.current_time() == 0);
			REQUIRE(in.view() == make_frame(shp, 0));
			
			REQUIRE(in.full_view_center() == 0);
			REQUIRE(in.full_view().shape()[0] == 1);
			REQUIRE(in.full_view()[0] == make_frame(shp, 0));	
			
			out.view() = in.view();	
		});
		graph.run_until(0);
		
	}
}
