#include <catch.hpp>
#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "support/graph.h"
#include "support/ndarray.h"
#include <thread>


using namespace mf;
using namespace mf::test;
using namespace std::literals;

TEST_CASE("media graph (parallel)", "[media_graph][parallel]") {
	media_graph graph;
	auto shp = make_ndsize(320, 240);
	
	SECTION("source --> passthrough(4) --> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source = graph.add_node<sequence_frame_source>(10, shp);
		auto& passthrough = graph.add_node<passthrough_node>(0, 0, 4);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		passthrough.input.connect(source.output);
		sink.input.connect(passthrough.output);
		graph.setup();
		graph.run();
		REQUIRE(sink.got_expected_frames());
	}

	SECTION("source --> passthrough(4) --> sink, stepwise") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source = graph.add_node<sequence_frame_source>(10, shp);
		auto& passthrough = graph.add_node<passthrough_node>(0, 0, 4);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		passthrough.input.connect(source.output);
		sink.input.connect(passthrough.output);
		graph.setup();
		
		REQUIRE(source.output.required_buffer_duration() == 1);
		REQUIRE(passthrough.output.required_buffer_duration() == 5);

		graph.run();
		REQUIRE(sink.got_expected_frames());
	}
	
	SECTION("source1 --> [+3]passthrough1(4) --> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source1 = graph.add_node<sequence_frame_source>(10, shp);
		auto& passthrough1 = graph.add_node<passthrough_node>(0, 3, 4);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		graph.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough1.offset() == 4);
		REQUIRE(source1.offset() == 3+4);
		
		REQUIRE(passthrough1.output.required_buffer_duration() == 5);
		REQUIRE(source1.output.required_buffer_duration() == 4);

		graph.run();
		
		REQUIRE(graph.current_time() == 10);
		REQUIRE(sink.got_expected_frames());
	}
	
	
	SECTION("source1 --> [-3]passthrough1(4) --> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source1 = graph.add_node<sequence_frame_source>(10, shp);
		auto& passthrough1 = graph.add_node<passthrough_node>(3, 0, 4);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		graph.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough1.offset() == 4);
		REQUIRE(source1.offset() == 4);
		
		REQUIRE(passthrough1.output.required_buffer_duration() == 5);
		REQUIRE(source1.output.required_buffer_duration() == 4);

		graph.run();
				
		REQUIRE(graph.current_time() == 10);
		REQUIRE(sink.got_expected_frames());
	}
	
	
	SECTION("source1 --> [-3,+3]passthrough1(4) --> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source1 = graph.add_node<sequence_frame_source>(10, shp);
		auto& passthrough1 = graph.add_node<passthrough_node>(3, 3, 4);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		graph.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough1.offset() == 4);
		REQUIRE(source1.offset() == 3+4);
		
		REQUIRE(passthrough1.output.required_buffer_duration() == 5);
		REQUIRE(source1.output.required_buffer_duration() == 7);

		graph.run();
				
		REQUIRE(graph.current_time() == 10);
		REQUIRE(sink.got_expected_frames());
	}


	SECTION("source1 --> [-3,+1]passthrough1(4) --> [-2,+2]passthrough2(5) --> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source1 = graph.add_node<sequence_frame_source>(10, shp);
		auto& passthrough1 = graph.add_node<passthrough_node>(3, 1, 4);
		auto& passthrough2 = graph.add_node<passthrough_node>(2, 2, 5);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		passthrough2.input.connect(passthrough1.output);
		sink.input.connect(passthrough2.output);
		
		graph.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough2.offset() == 5);
		REQUIRE(passthrough1.offset() == 5+4+2);
		REQUIRE(source1.offset() == 5+4+2+1);
		
		REQUIRE(passthrough2.output.required_buffer_duration() == 1+5);
		REQUIRE(passthrough1.output.required_buffer_duration() == 5+4);
		REQUIRE(source1.output.required_buffer_duration() == 5);

		graph.run();
		
		REQUIRE(graph.current_time() == 10);
		REQUIRE(sink.got_expected_frames());
	}	
}
