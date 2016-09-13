#include <catch.hpp>
#include <mf/filter/filter_graph.h>
#include <mf/filter/filter.h>
#include <mf/filter/filter_parameter.h>
#include <mf/utility/misc.h>
#include "../support/flow_parameter.h"

using namespace mf;
using namespace mf::test;



TEST_CASE("flow graph with parameters", "[flow][parameter]") {
	flow::filter_graph gr;
	auto shp = make_ndsize(10, 10);

	std::size_t count = 20;
	std::size_t last = count - 1;
	std::vector<int> seq(count);
	for(int i = 0; i < count; ++i) seq[i] = i;

	SECTION("deterministic") {
		auto& source = gr.add_filter<sequence_frame_source>(last, shp, true);
		auto& pass1 = gr.add_filter<parameter_passthrough_filter>(0, 0);
		auto& pass2 = gr.add_filter<parameter_passthrough_filter>(0, 0);
		auto& sink = gr.add_filter<expected_frames_sink>(seq);
		
		sink.input.connect(pass2.output);
		pass2.input.connect(pass1.output);
		pass1.input.connect(source.output);
		
		gr.setup();
		gr.run();
	}
}


