#include <catch.hpp>
#include "../src/flow/graph.h"
#include "../src/flow/node.h"
#include "support/flow.h"
#include "support/ndarray.h"
#include <iostream>

using namespace mf;
using namespace mf::test;

TEST_CASE("flow graph seekable", "[flow_graph][seek]") {
	//set_debug_mode(debug_mode::cerr);
	
	flow::graph gr;
	auto shp = make_ndsize(320, 240);

	std::vector<int> seq(20);
	for(int i = 0; i < seq.size(); ++i) seq[i] = i;	

	SECTION("source -> sink") {
		auto& source = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& sink = gr.add_sink<expected_frames_sink>(seq);
		sink.input.connect(source.output);
		gr.setup();
		gr.run();
		REQUIRE(sink.check());
	}

		
	SECTION("source --> passthrough --> sink") {
		auto& source = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough = gr.add_node<passthrough_node>(0, 0);
		auto& sink = gr.add_sink<expected_frames_sink>(seq);
		passthrough.input.connect(source.output);
		sink.input.connect(passthrough.output);
		gr.setup();
		gr.run();
		REQUIRE(sink.check());
	}
	
	SECTION("detailled time window test") { 
 		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5 };
		auto& source = gr.add_node<sequence_frame_source>(5, shp, true);		
		auto& sink = gr.add_sink<expected_frames_sink>(seq);


		SECTION("source --> [-3]pass --> sink") {
			auto& node = gr.add_node<passthrough_node>(3, 0);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			gr.setup();
				
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				switch(self.current_time()) {
				case 0:
					REQUIRE(in.view() == make_frame(shp, 0));
					REQUIRE(in.full_view_center() == 0);
					REQUIRE(compare_frames(shp, in.full_view(), { 0 }));
					break;

				case 1:
					REQUIRE(in.view() == make_frame(shp, 1));
					REQUIRE(in.full_view_center() == 1);
					REQUIRE(compare_frames(shp, in.full_view(), { 0, 1 }));
					break;

				case 2:
					REQUIRE(in.view() == make_frame(shp, 2));
					REQUIRE(in.full_view_center() == 2);
					REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2 }));
					break;
		
				case 3:
					REQUIRE(in.view() == make_frame(shp, 3));
					REQUIRE(in.full_view_center() == 3);
					REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3 }));
					break;
					
				case 4:
					REQUIRE(in.view() == make_frame(shp, 4));
					REQUIRE(in.full_view_center() == 3);
					REQUIRE(compare_frames(shp, in.full_view(), { 1, 2, 3, 4 }));
					break;
				
				case 5:
					REQUIRE(in.view() == make_frame(shp, 5));
					REQUIRE(in.full_view_center() == 3);
					REQUIRE(compare_frames(shp, in.full_view(), { 2, 3, 4, 5 }));
					break;
				}
			});

			gr.run();
			
			REQUIRE(sink.check());	
		}
	

		SECTION("source --> [+3]pass --> sink") {
			auto& node = gr.add_node<passthrough_node>(0, 3);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			gr.setup();
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				switch(self.current_time()) {
				case 0:
					REQUIRE(in.view() == make_frame(shp, 0));
					REQUIRE(in.full_view_center() == 0);
					REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3 }));
					break;

				case 1:
					REQUIRE(in.view() == make_frame(shp, 1));
					REQUIRE(in.full_view_center() == 0);
					REQUIRE(compare_frames(shp, in.full_view(), { 1, 2, 3, 4 }));
					break;

				case 2:
					REQUIRE(in.view() == make_frame(shp, 2));
					REQUIRE(in.full_view_center() == 0);
					REQUIRE(compare_frames(shp, in.full_view(), { 2, 3, 4, 5 }));
					break;
		
				case 3:
					REQUIRE(in.view() == make_frame(shp, 3));
					REQUIRE(in.full_view_center() == 0);
					REQUIRE(compare_frames(shp, in.full_view(), { 3, 4, 5 }));
					break;
					
				case 4:
					REQUIRE(in.view() == make_frame(shp, 4));
					REQUIRE(in.full_view_center() == 0);
					REQUIRE(compare_frames(shp, in.full_view(), { 4, 5 }));
					break;
				
				case 5:
					REQUIRE(in.view() == make_frame(shp, 5));
					REQUIRE(in.full_view_center() == 0);
					REQUIRE(compare_frames(shp, in.full_view(), { 5 }));
					break;
				}
			});
			
			gr.run();
			
			REQUIRE(sink.check());
		}


		SECTION("source --> [-3,+3]pass --> sink") {
			auto& node = gr.add_node<passthrough_node>(3, 3);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			gr.setup();
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				switch(self.current_time()) {
				case 0:
					REQUIRE(in.view() == make_frame(shp, 0));
					REQUIRE(in.full_view_center() == 0);
					REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3 }));
					break;
	
				case 1:
					REQUIRE(in.view() == make_frame(shp, 1));
					REQUIRE(in.full_view_center() == 1);
					REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3, 4 }));
					break;
	
				case 2:
					REQUIRE(in.view() == make_frame(shp, 2));
					REQUIRE(in.full_view_center() == 2);
					REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3, 4, 5 }));
					break;
		
				case 3:
					REQUIRE(in.view() == make_frame(shp, 3));
					REQUIRE(in.full_view_center() == 3);
					REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3, 4, 5 }));
					break;
					
				case 4:
					REQUIRE(in.view() == make_frame(shp, 4));
					REQUIRE(in.full_view_center() == 3);
					REQUIRE(compare_frames(shp, in.full_view(), { 1, 2, 3, 4, 5 }));
					break;
				
				case 5:
					REQUIRE(in.view() == make_frame(shp, 5));
					REQUIRE(in.full_view_center() == 3);
					REQUIRE(compare_frames(shp, in.full_view(), { 2, 3, 4, 5 }));
					break;
				}
			});

			gr.run();
			REQUIRE(sink.check());
		}
	}
	

	SECTION("source1 --> [+3]passthrough1 --> sink") {
		auto& source1 = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough1 = gr.add_node<passthrough_node>(0, 3);
		auto& sink = gr.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		gr.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough1.offset() == 0);
		REQUIRE(source1.offset() == 3);
		
		REQUIRE(passthrough1.output.required_buffer_duration() == 1);
		REQUIRE(source1.output.required_buffer_duration() == 4);

		gr.run();
		
		REQUIRE(gr.current_time() == seq.size()-1);
		REQUIRE(sink.check());
	}
	
	
	SECTION("source1 --> [-3]passthrough1 --> sink") {
		auto& source1 = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough1 = gr.add_node<passthrough_node>(3, 0);
		auto& sink = gr.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		gr.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough1.offset() == 0);
		REQUIRE(source1.offset() == 0);
		
		REQUIRE(passthrough1.output.required_buffer_duration() == 1);
		REQUIRE(source1.output.required_buffer_duration() == 4);

		gr.run();
				
		REQUIRE(gr.current_time() == seq.size()-1);
		REQUIRE(sink.check());
	}
	
	SECTION("source1 --> [-3,+3]passthrough1 --> sink") {
		auto& source1 = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough1 = gr.add_node<passthrough_node>(3, 3);
		auto& sink = gr.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		gr.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough1.offset() == 0);
		REQUIRE(source1.offset() == 3);
		
		REQUIRE(passthrough1.output.required_buffer_duration() == 1);
		REQUIRE(source1.output.required_buffer_duration() == 7);

		gr.run();
				
		REQUIRE(gr.current_time() == seq.size()-1);
		REQUIRE(sink.check());
	}

	SECTION("source1 --> [-3,+1]passthrough1 --> [-2,+2]passthrough2 --> sink") {
		auto& source1 = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough1 = gr.add_node<passthrough_node>(3, 1);
		auto& passthrough2 = gr.add_node<passthrough_node>(2, 2);
		auto& sink = gr.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		passthrough2.input.connect(passthrough1.output);
		sink.input.connect(passthrough2.output);
		
		gr.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough2.offset() == 0);
		REQUIRE(passthrough1.offset() == 2);
		REQUIRE(source1.offset() == 3);
		
		REQUIRE(passthrough2.output.required_buffer_duration() == 1);
		REQUIRE(passthrough1.output.required_buffer_duration() == 5);
		REQUIRE(source1.output.required_buffer_duration() == 5);

		gr.run();
		
		REQUIRE(gr.current_time() == seq.size()-1);
		REQUIRE(sink.check());
	}
	
	
	SECTION("input synchronize") {
		auto& source1 = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& source2 = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& sink = gr.add_sink<expected_frames_sink>(seq);
		auto& merge = gr.add_node<input_synchronize_test_node>();
		
		SECTION("graph 1") {
			/*
			source1 --> [+5]passthrough --> merge --> sink
			source2 ----------------------> /
			*/
			
			auto& passthrough = gr.add_node<passthrough_node>(0, 5);
		
			passthrough.input.connect(source1.output);
			merge.input1.connect(passthrough.output);
			merge.input2.connect(source2.output);
			sink.input.connect(merge.output);
		
			gr.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 0);
			REQUIRE(passthrough.offset() == 0);
			REQUIRE(source1.offset() == 5);
			REQUIRE(source2.offset() == 0);
			
			REQUIRE(merge.output.required_buffer_duration() == 1);
			REQUIRE(passthrough.output.required_buffer_duration() == 1);
			REQUIRE(source1.output.required_buffer_duration() == 6); // current + 5 future
			REQUIRE(source2.output.required_buffer_duration() == 1);
			
			gr.run();
			
			REQUIRE(! merge.failed());
			REQUIRE(gr.current_time() == seq.size()-1);
			REQUIRE(sink.check());
		}

		SECTION("graph 2") {
			/*
			source1 --> [-3, +1]passthrough1 --> merge --> sink
			source2 --> [-1, +2]passthrough2 --> /
			*/
			
			auto& passthrough1 = gr.add_node<passthrough_node>(3, 1);
			auto& passthrough2 = gr.add_node<passthrough_node>(1, 2);
		
			passthrough1.input.connect(source1.output);
			passthrough2.input.connect(source2.output);
			merge.input1.connect(passthrough1.output);
			merge.input2.connect(passthrough2.output);
			sink.input.connect(merge.output);
		
			gr.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 0);
			REQUIRE(passthrough1.offset() == 0);
			REQUIRE(passthrough2.offset() == 0);
			REQUIRE(source1.offset() == 1);
			REQUIRE(source2.offset() == 2);
			
			REQUIRE(merge.output.required_buffer_duration() == 1);
			REQUIRE(passthrough1.output.required_buffer_duration() == 1);
			REQUIRE(passthrough2.output.required_buffer_duration() == 1);
			REQUIRE(source1.output.required_buffer_duration() == 5);
			REQUIRE(source2.output.required_buffer_duration() == 4);
			
			gr.run();
			
			REQUIRE(! merge.failed());
			REQUIRE(gr.current_time() == seq.size()-1);
			REQUIRE(sink.check());
		}
	}
	
	
	SECTION("multiple outputs") {
		auto& source = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& merge = gr.add_node<input_synchronize_test_node>();
		auto& multiplex = gr.add_node<multiplexer_node>();
		auto& sink = gr.add_sink<expected_frames_sink>(seq);

		SECTION("graph 1") {
			/*
			source --> multiplex --> merge --> sink
			                   \ --> /
			*/
			
			multiplex.input.connect(source.output);
			merge.input1.connect(multiplex.output1);
			merge.input2.connect(multiplex.output2);
			sink.input.connect(merge.output);
			
			gr.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 0);
			REQUIRE(multiplex.offset() == 0);
			REQUIRE(source.offset() == 0);

			REQUIRE(merge.output.required_buffer_duration() == 1);
			REQUIRE(multiplex.output1.required_buffer_duration() == 1);
			REQUIRE(multiplex.output2.required_buffer_duration() == 1);
			REQUIRE(source.output.required_buffer_duration() == 1);
			
			gr.run();

			REQUIRE(! merge.failed());
			REQUIRE(gr.current_time() == seq.size()-1);
			REQUIRE(sink.check());		
		}
		
		SECTION("graph 2") {
			/*
			source --> multiplex --------------------------------------------> merge --> sink
			                   \ --> [+5]passthrough1 --> [+3]passthrough2 --> /
			*/
			
			auto& passthrough1 = gr.add_node<passthrough_node>(0, 5);
			auto& passthrough2 = gr.add_node<passthrough_node>(0, 3);

			multiplex.input.connect(source.output);
			merge.input1.connect(multiplex.output1);
			passthrough1.input.connect(multiplex.output2);
			passthrough2.input.connect(passthrough1.output);
			merge.input2.connect(passthrough2.output);
			sink.input.connect(merge.output);
			
			gr.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 0);
			REQUIRE(passthrough2.offset() == 0);
			REQUIRE(passthrough1.offset() == 3);
			REQUIRE(multiplex.offset() == 3+5);
			REQUIRE(source.offset() == 3+5);

			REQUIRE(merge.output.required_buffer_duration() == 1);
			REQUIRE(passthrough2.output.required_buffer_duration() == 1);
			REQUIRE(passthrough1.output.required_buffer_duration() == 1+3);
			REQUIRE(multiplex.output1.required_buffer_duration() == 1+3+5); // need to respect offset
			REQUIRE(multiplex.output2.required_buffer_duration() == 1+5);
			REQUIRE(source.output.required_buffer_duration() == 1);			
			
			gr.run();

			REQUIRE(! merge.failed());
			REQUIRE(gr.current_time() == seq.size()-1);
			REQUIRE(sink.check());
		}
		
		SECTION("graph 3") {
			/*
			source --> multiplex --> [-1,+1]passthrough3---------------------------> merge --> sink
			                   \ --> [-2,+5]passthrough1 --> [-1,+3]passthrough2 --> /
			*/
			
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
			
			gr.setup();			
			gr.run();

			REQUIRE(! merge.failed());
			REQUIRE(gr.current_time() == seq.size()-1);
			REQUIRE(sink.check());
		}
	}
}
