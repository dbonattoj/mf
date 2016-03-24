#include <catch.hpp>
#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "support/graph.h"
#include "support/ndarray.h"
#include <thread>
#include <numeric>

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
	

	SECTION("input synchronize") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source1 = graph.add_node<sequence_frame_source>(10, shp);
		auto& source2 = graph.add_node<sequence_frame_source>(10, shp);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		auto& merge = graph.add_node<input_synchronize_test_node>();
		
		SECTION("graph 1") {
			/*
			source1 --> [+5]passthrough(4) --> merge --> sink
			source2 -------------------------> /
			*/
			
			auto& passthrough = graph.add_node<passthrough_node>(0, 5, 4);
		
			passthrough.input.connect(source1.output);
			merge.input1.connect(passthrough.output);
			merge.input2.connect(source2.output);
			sink.input.connect(merge.output);
		
			graph.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 0);
			REQUIRE(passthrough.offset() == 4);
			REQUIRE(source1.offset() == 5+4);
			REQUIRE(source2.offset() == 0);
			
			REQUIRE(merge.output.required_buffer_duration() == 1);
			REQUIRE(passthrough.output.required_buffer_duration() == 1+4);
			REQUIRE(source1.output.required_buffer_duration() == 6);
			REQUIRE(source2.output.required_buffer_duration() == 1);
			
			graph.run();
			
			REQUIRE(! merge.failed());
			REQUIRE(graph.current_time() == 10);
			REQUIRE(sink.got_expected_frames());
		}

		SECTION("graph 2") {
			/*
			source1 --> [-3, +1]passthrough1(4) --> merge --> sink
			source2 --> [-1, +2]passthrough2(3) --> /
			*/
			
			auto& passthrough1 = graph.add_node<passthrough_node>(3, 1, 4);
			auto& passthrough2 = graph.add_node<passthrough_node>(1, 2, 3);
		
			passthrough1.input.connect(source1.output);
			passthrough2.input.connect(source2.output);
			merge.input1.connect(passthrough1.output);
			merge.input2.connect(passthrough2.output);
			sink.input.connect(merge.output);
		
			graph.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 0);
			REQUIRE(passthrough1.offset() == 4);
			REQUIRE(passthrough2.offset() == 3);
			REQUIRE(source1.offset() == 1+4);
			REQUIRE(source2.offset() == 2+3);
			
			REQUIRE(merge.output.required_buffer_duration() == 1);
			REQUIRE(passthrough1.output.required_buffer_duration() == 1+4);
			REQUIRE(passthrough2.output.required_buffer_duration() == 1+3);
			REQUIRE(source1.output.required_buffer_duration() == 5);
			REQUIRE(source2.output.required_buffer_duration() == 4);
			
			graph.run();
			
			REQUIRE(! merge.failed());
			REQUIRE(graph.current_time() == 10);
			REQUIRE(sink.got_expected_frames());
		}
	}


	SECTION("multiple outputs") {
		std::vector<int> seq(20);
		for(int i = 0; i < seq.size(); ++i) seq[i] = i;
		auto& source = graph.add_node<sequence_frame_source>(seq.size()-1, shp);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		SECTION("graph 1, merge prefetch") {
			/*
			source --> multiplex --> merge(4) --> sink
			                   \ --> /
			*/

			auto& merge = graph.add_node<input_synchronize_test_node>(4);
			auto& multiplex = graph.add_node<multiplexer_node>();
			
			multiplex.input.connect(source.output);
			merge.input1.connect(multiplex.output1);
			merge.input2.connect(multiplex.output2);
			sink.input.connect(merge.output);
			
			graph.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 4);
			REQUIRE(multiplex.offset() == 4);
			REQUIRE(source.offset() == 4);

			REQUIRE(merge.output.required_buffer_duration() == 1+4);
			REQUIRE(multiplex.output1.required_buffer_duration() == 1);
			REQUIRE(multiplex.output2.required_buffer_duration() == 1);
			REQUIRE(source.output.required_buffer_duration() == 1);
			
			graph.run();

			REQUIRE(! merge.failed());
			REQUIRE(graph.current_time() == seq.size()-1);
			REQUIRE(sink.got_expected_frames());		
		}
		
		SECTION("graph 1, multiplex prefetch") {
			/*
			source --> multiplex(4) --> merge --> sink
			                      \ --> /
			*/

			auto& merge = graph.add_node<input_synchronize_test_node>();
			auto& multiplex = graph.add_node<multiplexer_node>(4);
			
			multiplex.input.connect(source.output);
			merge.input1.connect(multiplex.output1);
			merge.input2.connect(multiplex.output2);
			sink.input.connect(merge.output);
			
			graph.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 0);
			REQUIRE(multiplex.offset() == 4);
			REQUIRE(source.offset() == 4);

			REQUIRE(merge.output.required_buffer_duration() == 1);
			REQUIRE(multiplex.output1.required_buffer_duration() == 1+4);
			REQUIRE(multiplex.output2.required_buffer_duration() == 1+4);
			REQUIRE(source.output.required_buffer_duration() == 1);
			
			graph.run();

			REQUIRE(! merge.failed());
			REQUIRE(graph.current_time() == seq.size()-1);
			REQUIRE(sink.got_expected_frames());		
		}


		SECTION("graph 1, merge+multiplex prefetch") {
			/*
			source --> multiplex(4) --> merge(3) --> sink
			                      \ --> /
			*/

			auto& merge = graph.add_node<input_synchronize_test_node>(3);
			auto& multiplex = graph.add_node<multiplexer_node>(4);
			
			multiplex.input.connect(source.output);
			merge.input1.connect(multiplex.output1);
			merge.input2.connect(multiplex.output2);
			sink.input.connect(merge.output);
			
			graph.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 3);
			REQUIRE(multiplex.offset() == 3+4);
			REQUIRE(source.offset() == 3+4);

			REQUIRE(merge.output.required_buffer_duration() == 1+3);
			REQUIRE(multiplex.output1.required_buffer_duration() == 1+4);
			REQUIRE(multiplex.output2.required_buffer_duration() == 1+4);
			REQUIRE(source.output.required_buffer_duration() == 1);
			
			graph.run();

			REQUIRE(! merge.failed());
			REQUIRE(graph.current_time() == seq.size()-1);
			REQUIRE(sink.got_expected_frames());		
		}


		SECTION("graph 2") {
			/*
			source --> multiplex(1) --------------------------------------------------> merge(1) --> sink
			                      \ --> [+5]passthrough1(1) --> [+3]passthrough2(2) --> /
			*/
			
			auto& merge = graph.add_node<input_synchronize_test_node>(1);
			auto& multiplex = graph.add_node<multiplexer_node>(1);
			auto& passthrough1 = graph.add_node<passthrough_node>(0, 5, 1);
			auto& passthrough2 = graph.add_node<passthrough_node>(0, 3, 2);

			multiplex.input.connect(source.output);
			merge.input1.connect(multiplex.output1);
			passthrough1.input.connect(multiplex.output2);
			passthrough2.input.connect(passthrough1.output);
			merge.input2.connect(passthrough2.output);
			sink.input.connect(merge.output);
			
			graph.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 1);
			REQUIRE(passthrough2.offset() == 1+2);
			REQUIRE(passthrough1.offset() == 1+2+1+3);
			REQUIRE(multiplex.offset() == 1+2+1+3+5+1);
			REQUIRE(source.offset() == 1+2+1+3+5+1);
			
			graph.run();

			REQUIRE(! merge.failed());
			REQUIRE(graph.current_time() == seq.size()-1);
			REQUIRE(sink.got_expected_frames());
		}
		
		SECTION("graph 3") {
			/*
			source --> multiplex(1) --> [-1,+1]passthrough3(6)------------------------------> merge(2) --> sink
			                      \ --> [-2,+5]passthrough1(3) --> [-1,+3]passthrough2(1) --> /
			*/
			
			auto& merge = graph.add_node<input_synchronize_test_node>(2);
			auto& multiplex = graph.add_node<multiplexer_node>(1);
			auto& passthrough1 = graph.add_node<passthrough_node>(2, 5, 3);
			auto& passthrough2 = graph.add_node<passthrough_node>(1, 3, 1);
			auto& passthrough3 = graph.add_node<passthrough_node>(1, 1, 6);

			multiplex.input.connect(source.output);
			passthrough3.input.connect(multiplex.output1);
			merge.input1.connect(passthrough3.output);
			passthrough1.input.connect(multiplex.output2);
			passthrough2.input.connect(passthrough1.output);
			merge.input2.connect(passthrough2.output);
			sink.input.connect(merge.output);
			
			graph.setup();			
			graph.run();

			REQUIRE(! merge.failed());
			REQUIRE(graph.current_time() == seq.size()-1);
			REQUIRE(sink.got_expected_frames());
		}
	}
}
