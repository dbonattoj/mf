#include <catch.hpp>
#include "../src/flow/graph.h"
#include "../src/flow/sync_node.h"
#include "support/ndarray.h"
#include "support/flow.h"

using namespace mf;
using namespace mf::test;

using source_node_type = flow::sync_source_node;
using node_type = flow::sync_node;
constexpr bool seekable = false;

TEST_CASE("flow graph test: non-seekable, sync", "[flow_graph]" "[sync][nonseekable]") {
	#include "flow_tests_basic.icc"
}
