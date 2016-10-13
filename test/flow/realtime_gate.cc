/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <catch.hpp>
#include <mf/filter/filter_graph.h>
#include <mf/filter/filter.h>
#include <mf/filter/filter_parameter.h>
#include <mf/utility/misc.h>
#include <string>
#include "../support/flow_parameter.h"

using namespace mf;
using namespace mf::test;



TEST_CASE("flow graph with realtime gate", "[flow][realtime][gate]") {
	flow::filter_graph gr;
	auto shp = make_ndsize(10, 10);

	std::size_t count = 20;
	std::size_t last = count - 1;
/*
	SECTION("basic") {
		auto& source = gr.add_filter<sequence_frame_source>(last, shp);
		auto& sink = gr.add_filter<simple_sink>();
		
		sink->input.connect(source->output);

		sink.set_own_timing(flow::stream_timing::real_time());
		
		gr.setup();
		gr.run();
	}

	SECTION("pipeline") {
		auto& source = gr.add_filter<sequence_frame_source>(last, shp);
		auto& pass1 = gr.add_filter<passthrough_filter>(0, 0);
		auto& pass2 = gr.add_filter<passthrough_filter>(0, 0);
		auto& sink = gr.add_filter<simple_sink>();
		
		sink->input.connect(pass2->output);
		pass2->input.connect(pass1->output);
		pass1->input.connect(source->output);

		pass2.set_own_timing(flow::stream_timing::real_time());
		
		gr.setup();
		//gr.run();
	}
	*/
	SECTION("multiple gate edges") {
		auto& source = gr.add_filter<sequence_frame_source>(last, shp);
		auto& split = gr.add_filter<multiple_output_filter>();
		auto& merge = gr.add_filter<input_synchronize_test_filter>();
		auto& sink = gr.add_filter<simple_sink>();

		sink->input.connect(merge->output);
		merge->input1.connect(split->output1);
		merge->input2.connect(split->output2);
		split->input.connect(source->output);

		merge.set_own_timing(flow::stream_timing::real_time());
		
		gr.setup();
	}
}


