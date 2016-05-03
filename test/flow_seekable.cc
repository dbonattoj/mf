#include <catch.hpp>
#include "../src/flow/graph.h"
#include "../src/flow/async_node.h"
#include "support/flow.h"
#include "support/ndarray.h"
#include <iostream>

using namespace mf;
using namespace mf::test;

TEST_CASE("flow graph seekable", "[flow_graph][seek]") {
	set_debug_mode(debug_mode::cerr);
	set_debug_filter({"node"});
	
	flow::graph gr;
	auto shp = make_ndsize(10, 10);

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

	/*
	SECTION("detailled time window test") { 
 		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5 };
		auto& source = gr.add_node<sequence_frame_source>(5, shp, true);		
		auto& sink = gr.add_sink<expected_frames_sink>(seq);


		SECTION("source --> [-3]pass --> sink") {
			auto& node = gr.add_node<passthrough_node>(3, 0);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			gr.setup();
				
			node.callback = [&](passthrough_node& self, auto& in, auto& out) {
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
			};

			gr.run();
			
			REQUIRE(sink.check());	
		}
	

		SECTION("source --> [+3]pass --> sink") {
			auto& node = gr.add_node<passthrough_node>(0, 3);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			gr.setup();
			
			node.callback = [&](passthrough_node& self, auto& in, auto& out) {
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
			};
			
			gr.run();
			
			REQUIRE(sink.check());
		}


		SECTION("source --> [-3,+3]pass --> sink") {
			auto& node = gr.add_node<passthrough_node>(3, 3);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			gr.setup();
			
			node.callback = [&](passthrough_node& self, auto& in, auto& out) {
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
			};

			gr.run();
			REQUIRE(sink.check());
		}
	}
	*/

	SECTION("source1 --> [+3]passthrough1 --> sink") {
		auto& source1 = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough1 = gr.add_node<passthrough_node>(0, 3);
		auto& sink = gr.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		gr.setup();
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

		gr.run();
		
		REQUIRE(gr.current_time() == seq.size()-1);
		REQUIRE(sink.check());
	}
	
	SECTION("multiple inputs") {
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
			
			gr.run();
			
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
			
			gr.run();
			
			REQUIRE(gr.current_time() == seq.size()-1);
			REQUIRE(sink.check());
		}
		
		SECTION("graph 3") {
			/*
			source1 --> [-3, +1]passthrough1 ---------------------------> merge --> sink
			source2 --> [-2, +2]passthrough3 --> [-1, +2]passthrough2 --> /
			*/
			
			auto& passthrough1 = gr.add_node<passthrough_node>(3, 1);
			auto& passthrough2 = gr.add_node<passthrough_node>(1, 2);
			auto& passthrough3 = gr.add_node<passthrough_node>(2, 2);
		
			passthrough1.input.connect(source1.output);
			passthrough3.input.connect(source2.output);
			passthrough2.input.connect(passthrough3.output);
			merge.input1.connect(passthrough1.output);
			merge.input2.connect(passthrough2.output);
			sink.input.connect(merge.output);
		
			gr.setup();			
			gr.run();
			
			REQUIRE(gr.current_time() == seq.size()-1);
			REQUIRE(sink.check());
		}
	}
	

	SECTION("seek") {
		constexpr int m = missingframe;
		std::vector<int> seq { 0, 1, 2, 3, 4, 5, m, 7, 8, m, 10, m, m, m, m, 15, 16, m, m, 19 };

		SECTION("source -> sink") {			
			auto& source = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
			auto& sink = gr.add_sink<expected_frames_sink>(seq);
			sink.input.connect(source.output);
			gr.setup();
			gr.run_for(3);
			REQUIRE(gr.current_time() == 2);
			gr.run_until(5);
			REQUIRE(gr.current_time() == 5);
			
			// seek forward, skipping frames [6,9]
			gr.seek(15);
			gr.run_for(2);
			REQUIRE(gr.current_time() == 16);
			
			// seek backward
			gr.seek(10);
			gr.run_for(1);
			REQUIRE(gr.current_time() == 10);
			
			// invalid seek
			REQUIRE_THROWS(gr.seek(-1));
			REQUIRE(gr.current_time() == 10);
			REQUIRE_THROWS(gr.seek(20));
			REQUIRE(gr.current_time() == 10);	
			
			// seek to end
			gr.seek(19);
			gr.run();
			REQUIRE(sink.reached_end());
			
			// seek backward, after end was already reached
			gr.seek(7);
			gr.run_for(2);
			
			// stop, when not at end
			
			REQUIRE(sink.check());
		}

		SECTION("graph 2") {
			/*
			source1 --> [-3, +1]passthrough1 ---------------------------> merge --> sink
			source2 --> [-2, +2]passthrough3 --> [-1, +2]passthrough2 --> /
			*/

			auto& source1 = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
			auto& source2 = gr.add_node<sequence_frame_source>(seq.size()-1, shp, true);
			auto& sink = gr.add_sink<expected_frames_sink>(seq);
			auto& merge = gr.add_node<input_synchronize_test_node>();
			auto& passthrough1 = gr.add_node<passthrough_node>(3, 1);
			auto& passthrough2 = gr.add_node<passthrough_node>(1, 2);
			auto& passthrough3 = gr.add_node<passthrough_node>(2, 2);
			passthrough1.input.connect(source1.output);
			passthrough3.input.connect(source2.output);
			passthrough2.input.connect(passthrough3.output);
			merge.input1.connect(passthrough1.output);
			merge.input2.connect(passthrough2.output);
			sink.input.connect(merge.output);
			gr.setup();
			
			gr.run_for(3);
			REQUIRE(gr.current_time() == 2);
			gr.run_until(5);
			REQUIRE(gr.current_time() == 5);
			
			// seek forward, skipping frames [6,9]
			gr.seek(15);
			gr.run_for(2);
			REQUIRE(gr.current_time() == 16);
			
			// seek backward
			gr.seek(10);
			gr.run_for(1);
			REQUIRE(gr.current_time() == 10);
			
			// invalid seek
			REQUIRE_THROWS(gr.seek(-1));
			REQUIRE(gr.current_time() == 10);
			REQUIRE_THROWS(gr.seek(20));
			REQUIRE(gr.current_time() == 10);	
			
			// seek to end
			gr.seek(19);
			gr.run();
			REQUIRE(sink.reached_end());
			
			// seek backward, after end was already reached
			gr.seek(7);
			gr.run_for(2);		
			
			// stop, when not at end
			// stop, when not at end
			REQUIRE(sink.check());
		}
	}
}
