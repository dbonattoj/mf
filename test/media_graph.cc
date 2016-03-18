#include <catch.hpp>
#include "../src/graph/media_graph.h"
#include "../src/graph/media_sequential_node.h"
#include "support/graph.h"
#include "support/ndarray.h"
#include <iostream>

using namespace mf;
using namespace mf::test;

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
	
	
	SECTION("source --> passthrough --> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source = graph.add_node<sequence_frame_source>(10, shp);
		auto& passthrough = graph.add_node<passthrough_node>(0, 0);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);
		passthrough.input.connect(source.output);
		passthrough.set_callback([](passthrough_node& self, auto& in, auto& out) {
			out.view() = in.view();
		});
		sink.input.connect(passthrough.output);
		graph.setup();
		graph.run();
		REQUIRE(sink.got_expected_frames());
	}
	
	
	SECTION("detailled time window test") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5 };
		auto& source = graph.add_node<sequence_frame_source>(5, shp);		
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		SECTION("source [-3]--> sink") {
			auto& node = graph.add_node<passthrough_node>(3, 0);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			graph.setup();
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 0);
				REQUIRE(in.view() == make_frame(shp, 0));
				
				REQUIRE(in.full_view_center() == 0);
				REQUIRE(compare_frames(shp, in.full_view(), { 0 }));
			});
			graph.run_until(0);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 1);
				REQUIRE(in.view() == make_frame(shp, 1));
				
				REQUIRE(in.full_view_center() == 1);
				REQUIRE(compare_frames(shp, in.full_view(), { 0, 1 }));
			});
			graph.run_until(1);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 2);
				REQUIRE(in.view() == make_frame(shp, 2));
				
				REQUIRE(in.full_view_center() == 2);
				REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2 }));
			});
			graph.run_until(2);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 3);
				REQUIRE(in.view() == make_frame(shp, 3));
				
				REQUIRE(in.full_view_center() == 3);
				REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3 }));
			});
			graph.run_until(3);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 4);
				REQUIRE(in.view() == make_frame(shp, 4));
				
				REQUIRE(in.full_view_center() == 3);
				REQUIRE(compare_frames(shp, in.full_view(), { 1, 2, 3, 4 }));
			});
			graph.run_until(4);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 5);
				REQUIRE(in.view() == make_frame(shp, 5));
				
				REQUIRE(in.full_view_center() == 3);
				REQUIRE(compare_frames(shp, in.full_view(), { 2, 3, 4, 5 }));
			});
			graph.run_until(5);		
		}
		
		SECTION("source [+3]--> sink") {
			auto& node = graph.add_node<passthrough_node>(0, 3);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			graph.setup();
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 0);
				REQUIRE(in.view() == make_frame(shp, 0));
				
				REQUIRE(in.full_view_center() == 0);
				REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3 }));
			});
			graph.run_until(0);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 1);
				REQUIRE(in.view() == make_frame(shp, 1));
				
				REQUIRE(in.full_view_center() == 0);
				REQUIRE(compare_frames(shp, in.full_view(), { 1, 2, 3, 4 }));
			});
			graph.run_until(1);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 2);
				REQUIRE(in.view() == make_frame(shp, 2));
				
				REQUIRE(in.full_view_center() == 0);
				REQUIRE(compare_frames(shp, in.full_view(), { 2, 3, 4, 5 }));
			});
			graph.run_until(2);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 3);
				REQUIRE(in.view() == make_frame(shp, 3));
				
				REQUIRE(in.full_view_center() == 0);
				REQUIRE(compare_frames(shp, in.full_view(), { 3, 4, 5 }));
			});
			graph.run_until(3);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 4);
				REQUIRE(in.view() == make_frame(shp, 4));
				
				REQUIRE(in.full_view_center() == 0);
				REQUIRE(compare_frames(shp, in.full_view(), { 4, 5 }));
			});
			graph.run_until(4);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 5);
				REQUIRE(in.view() == make_frame(shp, 5));
				
				REQUIRE(in.full_view_center() == 0);
				REQUIRE(compare_frames(shp, in.full_view(), { 5 }));
			});
			graph.run_until(5);	
		}
	
		SECTION("source [-3,+3]--> sink") {
			auto& node = graph.add_node<passthrough_node>(3, 3);
	
			node.input.connect(source.output);
			sink.input.connect(node.output);
			graph.setup();
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 0);
				REQUIRE(in.view() == make_frame(shp, 0));
				
				REQUIRE(in.full_view_center() == 0);
				REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3 }));
			});
			graph.run_until(0);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 1);
				REQUIRE(in.view() == make_frame(shp, 1));
				
				REQUIRE(in.full_view_center() == 1);
				REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3, 4 }));
			});
			graph.run_until(1);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 2);
				REQUIRE(in.view() == make_frame(shp, 2));
				
				REQUIRE(in.full_view_center() == 2);
				REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3, 4, 5 }));
			});
			graph.run_until(2);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 3);
				REQUIRE(in.view() == make_frame(shp, 3));
				
				REQUIRE(in.full_view_center() == 3);
				REQUIRE(compare_frames(shp, in.full_view(), { 0, 1, 2, 3, 4, 5 }));
			});
			graph.run_until(3);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 4);
				REQUIRE(in.view() == make_frame(shp, 4));
				
				REQUIRE(in.full_view_center() == 3);
				REQUIRE(compare_frames(shp, in.full_view(), { 1, 2, 3, 4, 5 }));
			});
			graph.run_until(4);
			
			node.set_callback([&](passthrough_node& self, auto& in, auto& out) {
				REQUIRE(self.current_time() == 5);
				REQUIRE(in.view() == make_frame(shp, 5));
				
				REQUIRE(in.full_view_center() == 3);
				REQUIRE(compare_frames(shp, in.full_view(), { 2, 3, 4, 5 }));
			});
			graph.run_until(5);	
		}
	}
	
	
	SECTION("source1 --> [+3]passthrough1 --> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source1 = graph.add_node<sequence_frame_source>(10, shp);
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
		
		REQUIRE(graph.current_time() == 10);
	}
	
	
	SECTION("source1 --> [-3]passthrough1 --> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source1 = graph.add_node<sequence_frame_source>(10, shp);
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
				
		REQUIRE(graph.current_time() == 10);
	}


/*
	SECTION("source1 --> [+1]passthrough1 --> [+1]passthrough2 --> sink") {
		const std::vector<int>& seq { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto& source1 = graph.add_node<sequence_frame_source>(10, shp);
		auto& passthrough1 = graph.add_node<passthrough_node>(0, 1);
		auto& passthrough2 = graph.add_node<passthrough_node>(0, 1);
		auto& sink = graph.add_sink<expected_frames_sink>(seq);

		passthrough1.input.connect(source1.output);
		passthrough2.input.connect(passthrough1.output);
		sink.input.connect(passthrough2.output);
		
		graph.setup();

		REQUIRE(sink.offset() == 0);
		REQUIRE(passthrough2.offset() == 0);
		REQUIRE(passthrough1.offset() == 1);
		REQUIRE(source1.offset() == 2);
		
		REQUIRE(passthrough2.output.required_buffer_duration() == 1);
		REQUIRE(passthrough1.output.required_buffer_duration() == 2);
		REQUIRE(source1.output.required_buffer_duration() == 2);

		graph.run();
		
		REQUIRE(graph.current_time() == 10);
	}	
	
	
	
	*/
	
	
	
	
	
	
	
	
	
	
	
/*	
	
	SECTION("input synchronize") {
		class test_node : public media_sequential_node {
		public:
			media_node_input<2, int> input1;
			media_node_input<2, int> input2;
			media_node_output<2, int> output;
			
			test_node() :
				input1(*this), input2(*this), output(*this) { }
		
			void setup_() override {
				output.define_frame_shape(input1.frame_shape());
			}
			
			void process_() override {
				// test time synchronization:
				// frames on both inputs must be same
				
				REQUIRE(input1.view() == input2.view());
				output.view() = input1.view();
			}
		};
		
		
		
		SECTION("graph1") {
			/ *
			source1 -->[-3, +3] passthrough --> merge --> sink
			source2 --------------------------> /
			* /
			
			auto& source1 = graph.add_node<sequence_frame_source>(10, shp);
			auto& source2 = graph.add_node<sequence_frame_source>(10, shp);
			auto& passthrough = graph.add_node<passthrough_node>(3, 3);
			auto& merge = graph.add_node<test_node>();
			auto& sink = graph.add_sink<expected_frames_sink>(seq);
		
			passthrough.input.connect(source1.output);
			merge.input1.connect(passthrough.output);
			merge.input2.connect(source2.output);
			sink.input.connect(merge.output);
		
			graph.setup();
			
			REQUIRE(sink.offset() == 0);
			REQUIRE(merge.offset() == 0);
			REQUIRE(passthrough.offset() == 0);
			REQUIRE(source1.offset() == 3);
			REQUIRE(source2.offset() == 0);
			
			REQUIRE(merge.output.required_buffer_duration() == 1);
			REQUIRE(passthrough.output.required_buffer_duration() == 1);
			REQUIRE(source1.output.required_buffer_duration() == 7); // current + 3 past + 3 future
			REQUIRE(source2.output.required_buffer_duration() == 1);
			
			graph.run();
			
			REQUIRE(graph.current_time() == 10);
		}
		
		

		
		SECTION("graph2") {
			/ *
			source1 -->[-3, +1] passthrough1 --> merge --> sink
			source2 -->[-1, +2] passthrough2 --> /
			* /
			
			auto& passthrough1 = graph.add_node<passthrough_node>(3, 1);
			auto& passthrough2 = graph.add_node<passthrough_node>(1, 2);
			auto& merge = graph.add_node<test_node>();
			auto& sink = graph.add_sink<expected_frames_sink>(seq);
		
			passthrough1.input.connect(source1.output);
			passthrough2.input.connect(source2.output);
			merge.input1.connect(passthrough1.output);
			merge.input2.connect(passthrough2.output);
			sink.input.connect(merge.output);
		
			graph.setup();
			
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
			
			// passthrough1 fails
			graph.run();

			REQUIRE(graph.current_time() == 10);
		}
	}
	
*/
}
