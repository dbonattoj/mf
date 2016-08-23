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
#include <mf/utility/misc.h>
#include "../support/ndarray.h"
#include "../support/flow.h"

#include <mf/flow/graph_visualization.h>

using namespace mf;
using namespace mf::test;

TEST_CASE("flow graph test: seekable, mixed", "[flow][sync+async][seekable]") {
	constexpr bool seekable = true;
	constexpr bool async1 = false;
	constexpr bool async2 = true;

	#include "flow_tests_basic.icc"
	#include "flow_tests_activation.icc"
	#include "flow_tests_seek.icc"
}

TEST_CASE("flow graph test: seekable, mixed alt.", "[flow][sync+async][seekable]") {
	constexpr bool seekable = true;
	constexpr bool async1 = true;
	constexpr bool async2 = false;

	#include "flow_tests_basic.icc"
	#include "flow_tests_activation.icc"
	#include "flow_tests_seek.icc"
}
