#include <catch.hpp>
#include "../src/ndarray.h"

using namespace mf;

TEST_CASE("ndarray", "[ndarray]") {
	ndarray<2, int> arr(ndsize<2>(10, 10));
}