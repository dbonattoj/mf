#include <catch.hpp>
#include "../src/flow/graph.h"
#include "../src/flow/async_node.h"
#include "support/ndarray.h"
#include "support/flow.h"

using namespace mf;
using namespace mf::test;

using node_type = flow::async_node;
constexpr bool seekable = true;

TEST_CASE("flow graph test: seekable, async", "[flow_graph]" "[async][seekable]") {
	#include "flow_tests_basic.icc"
	#include "flow_tests_seek.icc"
}
