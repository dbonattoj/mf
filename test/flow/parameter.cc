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



TEST_CASE("flow graph with parameters", "[flow][parameter]") {
	flow::filter_graph gr;
	auto shp = make_ndsize(10, 10);

	std::size_t count = 20;
	std::size_t last = count - 1;
	std::vector<int> seq(count);
	for(int i = 0; i < count; ++i) seq[i] = i;

/*
	SECTION("deterministic (constant)") {
		auto& source = gr.add_filter<sequence_frame_source>(last, shp, true);
		auto& pass1 = gr.add_filter<parameter_passthrough_filter>(0, 0);
		auto& pass2 = gr.add_filter<parameter_passthrough_filter>(0, 0);
		auto& sink = gr.add_filter<expected_frames_sink>(seq);
		
		pass1.input.connect(source.output);
		pass1.set_name("param orig");
		auto& par = pass1.add_param(false);
		par.set_constant_value("value");
		par.set_name("par");

		pass2.input.connect(pass1.output);
		pass2.set_name("param dest");
		auto& e_par = pass2.add_extern_param(false);
		e_par.link(par);
		
		pass2.set_expected_value(e_par, "value");
		
		sink.input.connect(pass2.output);
		
		gr.setup();
		gr.run();
	}
	
	SECTION("deterministic (value function)") {
		auto& source = gr.add_filter<sequence_frame_source>(last, shp, true);
		auto& pass1 = gr.add_filter<parameter_passthrough_filter>(0, 0);
		auto& pass2 = gr.add_filter<parameter_passthrough_filter>(0, 0);
		auto& sink = gr.add_filter<expected_frames_sink>(seq);
		
		pass1.input.connect(source.output);
		pass1.set_name("param orig");
		auto& par = pass1.add_param(false);
		par.set_value_function([](time_unit t)->std::string {
			return std::to_string(t);
		});
		par.set_name("par");

		pass2.input.connect(pass1.output);
		pass2.set_name("param dest");
		auto& e_par = pass2.add_extern_param(true);
		e_par.link(par);
		
		sink.input.connect(pass2.output);
		
		gr.setup();
		gr.run();
	}
*/

	SECTION("dynamic") {
		auto& source = gr.add_filter<sequence_frame_source>(last, shp, true);
		auto& pass1 = gr.add_filter<parameter_passthrough_filter>(0, 0);
		auto& pass2 = gr.add_filter<parameter_passthrough_filter>(0, 0);
		auto& sink = gr.add_filter<expected_frames_sink>(seq);
		
		pass1.input.connect(source.output);
		pass1.set_name("param orig");
		auto& par = pass1.add_param(true);
		par.set_dynamic();
		par.set_name("par");

		pass2.input.connect(pass1.output);
		pass2.set_name("param dest");
		auto& e_par = pass2.add_extern_param(true);
		e_par.link(par);
				
		sink.input.connect(pass2.output);
		
		gr.setup();
		gr.run();
	}
}


