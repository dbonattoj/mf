#include <catch.hpp>
#include "../src/flow/graph.h"
#include "../src/flow/async_node.h"
#include "support/ndarray.h"
#include "support/flow.h"

using namespace mf;
using namespace mf::test;

using source_node_type = flow::async_source_node;
using node_type = flow::async_node;
constexpr bool seekable = false;

TEST_CASE("flow graph test: non-seekable, async", "[flow_graph]" "[async][nonseekable]") {
	#include "flow_tests_basic.icc"
}
