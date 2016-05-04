#include <catch.hpp>
#include "../src/flow/graph.h"
#include "../src/flow/sync_node.h"
#include "support/ndarray.h"
#include "support/flow.h"

using namespace mf;
using namespace mf::test;

using source_node_type = flow::sync_source_node;
using node_type = flow::sync_node;

TEST_CASE("flow graph test: seekable, sync", "[flow_graph]" "[sync][seekable]") {
	#include "flow_tests.icc"
}
