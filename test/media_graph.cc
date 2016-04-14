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

	std::vector<int> seq(20);
	for(int i = 0; i < seq.size(); ++i) seq[i] = i;	

	set_debug_mode(debug_mode::file);

	SECTION("source -> sink") {
		auto& source = graph.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		sink.input.connect(source.output);
		graph.setup();
		graph.run();
		REQUIRE(sink.check());
	}


	SECTION("source --> passthrough --> sink") {
		auto& source = graph.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough = graph.add_node<passthrough_node>(0, 0);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		passthrough.input.connect(source.output);
		sink.input.connect(passthrough.output);
		graph.setup();
		graph.run();
		REQUIRE(sink.check());
	}

	SECTION("detailled time window test") { 
 		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5 };
		auto& source = graph.add_node<sequence_frame_source>(5, shp, true);		
		auto& sink = graph.add_sink<expected_frames_sink>(seq);


		SECTION("source [-3]--> pass --> sink") {
			auto& node = graph.add_node<passthrough_node>(3, 0);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			graph.setup();
				
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

			graph.run();
			
			REQUIRE(sink.check());	
		}
	

		SECTION("source [+3]--> pass --> sink") {
			auto& node = graph.add_node<passthrough_node>(0, 3);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			graph.setup();
			
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
			
			graph.run();
			
			REQUIRE(sink.check());
		}


		SECTION("source [-3,+3]--> sink") {
			auto& node = graph.add_node<passthrough_node>(3, 3);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			graph.setup();
			
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

			graph.run();
			REQUIRE(sink.check());
		}
	}
	

	SECTION("source1 --> [+3]passthrough1 --> sink") {
		auto& source1 = graph.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough1 = graph.add_node<passthrough_node>(0, 3);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		graph.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough1.offset() == 0);
		REQUIRE(source1.offset() == 3);
		
		REQUIRE(passthrough1.output.required_buffer_duration() == 1);
		REQUIRE(source1.output.required_buffer_duration() == 4);

		graph.run();
		
		REQUIRE(graph.current_time() == seq.size()-1);
		REQUIRE(sink.check());
	}
	
	
	SECTION("source1 --> [-3]passthrough1 --> sink") {
		auto& source1 = graph.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough1 = graph.add_node<passthrough_node>(3, 0);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		graph.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough1.offset() == 0);
		REQUIRE(source1.offset() == 0);
		
		REQUIRE(passthrough1.output.required_buffer_duration() == 1);
		REQUIRE(source1.output.required_buffer_duration() == 4);

		graph.run();
				
		REQUIRE(graph.current_time() == seq.size()-1);
		REQUIRE(sink.check());
	}
	
	SECTION("source1 --> [-3,+3]passthrough1 --> sink") {
		auto& source1 = graph.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough1 = graph.add_node<passthrough_node>(3, 3);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		sink.input.connect(passthrough1.output);
		
		graph.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough1.offset() == 0);
		REQUIRE(source1.offset() == 3);
		
		REQUIRE(passthrough1.output.required_buffer_duration() == 1);
		REQUIRE(source1.output.required_buffer_duration() == 7);

		graph.run();
				
		REQUIRE(graph.current_time() == seq.size()-1);
		REQUIRE(sink.check());
	}

	SECTION("source1 --> [-3,+1]passthrough1 --> [-2,+2]passthrough2 --> sink") {
		auto& source1 = graph.add_node<sequence_frame_source>(seq.size()-1, shp, true);
		auto& passthrough1 = graph.add_node<passthrough_node>(3, 1);
		auto& passthrough2 = graph.add_node<passthrough_node>(2, 2);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		passthrough2.input.connect(passthrough1.output);
		sink.input.connect(passthrough2.output);
		
		graph.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough2.offset() == 0);
		REQUIRE(passthrough1.offset() == 2);
		REQUIRE(source1.offset() == 3);
		
		REQUIRE(passthrough2.output.required_buffer_duration() == 1);
		REQUIRE(passthrough1.output.required_buffer_duration() == 5);
		REQUIRE(source1.output.required_buffer_duration() == 5);

		graph.run();
		
		REQUIRE(graph.current_time() == seq.size()-1);
		REQUIRE(sink.check());
	}

}
