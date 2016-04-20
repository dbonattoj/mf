#include <catch.hpp>
#include "../src/flow/graph.h"
#include "../src/flow/async_node.h"
#include "support/flow.h"
#include "support/ndarray.h"
#include "../src/utility/string.h"
#include <iostream>

using namespace mf;
using namespace mf::test;

TEST_CASE("flow graph activation", "[flow_graph][activation]") {
	//set_debug_mode(debug_mode::cerr);
	
	flow::graph gr;
	auto shp = make_ndsize(10, 10);
	constexpr int n = noframe;
	
	const std::vector<bool>& act1     { 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1,  1,  1 };
	const std::vector<int>& seq1      { 0, 1, 2, n, n, n, 6, 7, n, n, 10, 11, 12 };
	const std::vector<bool>& act2     { 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1,  0,  0 };
	const std::vector<int>& seq2      { 0, 1, n, n, 4, n, 6, 7, n, 9, 10, n , n };


	const std::vector<bool>& act_and  { 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1,  0,  0 };
	const std::vector<int>& seq_and   { 0, 1, n, n, n, n, 6, 7, n, n, 10, n,  n };

	const std::vector<int>& seq_or    { 0, 1, 2, n, 4, n, 6, 7, n, 9, 10, 11, 12 };
	const std::size_t len = seq_and.size() - 1;


	SECTION("source -> sink") {
		std::vector<int> seq(100, n);
		std::vector<bool> act(100, false);
		seq[0] = 0; act[0] = true;
		seq[99] = 99; act[99] = true;
		std::size_t len = 99;
		
		auto& source = gr.add_node<sequence_frame_source>(len, shp, true);
		auto& sink = gr.add_sink<expected_frames_sink>(seq);
		sink.input.connect(source.output);
		sink.activation = act;
		gr.setup();
		gr.run();
		REQUIRE(sink.check());
		REQUIRE(gr.current_time() == len);
	}

	SECTION("source -> sink") {
		auto& source = gr.add_node<sequence_frame_source>(len, shp, true);
		auto& sink = gr.add_sink<expected_frames_sink>(seq_and);
		sink.input.connect(source.output);
		sink.activation = act_and;
		gr.setup();
		gr.run();
		REQUIRE(sink.check());
		REQUIRE(gr.current_time() == len);
	}



	SECTION("source -> passthrough -> sink") {
		auto& source = gr.add_node<sequence_frame_source>(len, shp, true);
		auto& sink = gr.add_sink<expected_frames_sink>(seq_and);
		auto& passthrough = gr.add_node<passthrough_node>(0, 0);

		passthrough.input.connect(source.output);
		sink.input.connect(passthrough.output);
		passthrough.activation = act1;
		sink.activation = act2;
		gr.setup();
		gr.launch();
		gr.run();
		REQUIRE(sink.check());
		REQUIRE(gr.current_time() == len);
	}
	return;


	SECTION("source -> [-3,+3]passthrough -> sink") {
		auto& source = gr.add_node<sequence_frame_source>(len, shp, true);
		auto& sink = gr.add_sink<expected_frames_sink>(seq_and);
		auto& passthrough = gr.add_node<passthrough_node>(3, 3);
		passthrough.input.connect(source.output);
		sink.input.connect(passthrough.output);
		passthrough.activation = act1;
		sink.activation = act2;
		gr.setup();
		gr.launch();
		gr.run();
		REQUIRE(sink.check());
		REQUIRE(gr.current_time() == len);
	}

	SECTION("two inputs") {
		/*
		source1 --> [-3, +1]passthrough1 --> merge --> sink
		source2 --> [-1, +2]passthrough2 --> /
		*/
		
		auto& source1 = gr.add_node<sequence_frame_source>(len, shp, true);
		auto& source2 = gr.add_node<sequence_frame_source>(len, shp, true);
		auto& passthrough1 = gr.add_node<passthrough_node>(3, 1);
		auto& passthrough2 = gr.add_node<passthrough_node>(1, 2);
		auto& merge = gr.add_node<input_synchronize_test_node>();
		auto& sink = gr.add_sink<expected_frames_sink>(seq_or);

	
		passthrough1.input.connect(source1.output);
		passthrough2.input.connect(source2.output);
		merge.input1.connect(passthrough1.output);
		merge.input2.connect(passthrough2.output);
		sink.input.connect(merge.output);
	
		merge.activation1 = act1;
		merge.activation2 = act2;
	
		gr.setup();		
		gr.run();
		
		REQUIRE(sink.check());
	}


	SECTION("graph 3") {
		/*
		source --> multiplex --> [-1,+1]passthrough3---------------------------> merge --> sink
		                   \ --> [-2,+5]passthrough1 --> [-1,+3]passthrough2 --> /
		*/
		
		auto& source = gr.add_node<sequence_frame_source>(len, shp, true);
		auto& merge = gr.add_node<input_synchronize_test_node>();
		auto& multiplex = gr.add_node<multiplexer_node>();
		auto& sink = gr.add_sink<expected_frames_sink>(seq_or);		
		auto& passthrough1 = gr.add_node<passthrough_node>(2, 5);
		auto& passthrough2 = gr.add_node<passthrough_node>(1, 3);
		auto& passthrough3 = gr.add_node<passthrough_node>(1, 1);

		multiplex.input.connect(source.output);
		passthrough3.input.connect(multiplex.output1);
		merge.input1.connect(passthrough3.output);
		passthrough1.input.connect(multiplex.output2);
		passthrough2.input.connect(passthrough1.output);
		merge.input2.connect(passthrough2.output);
		sink.input.connect(merge.output);
		
		SECTION("merge inputs") {
			merge.activation1 = act1;
			merge.activation2 = act2;
			
			gr.setup();			
			gr.run();

			REQUIRE(gr.current_time() == len);
			REQUIRE(sink.check());
		}
		
		SECTION("passthroughs 1, 3") {
			passthrough1.activation = act1;
			passthrough3.activation = act2;

			gr.setup();			
			gr.run();

			REQUIRE(gr.current_time() == len);
			REQUIRE(sink.check());
		}
		
		SECTION("passthroughs 2, 3") {
			passthrough2.activation = act1;
			passthrough3.activation = act2;

			gr.setup();			
			gr.run();

			REQUIRE(gr.current_time() == len);
			REQUIRE(sink.check());
		}	
	}
}
		
