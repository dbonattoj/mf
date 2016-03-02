#include <catch.hpp>
#include "../src/ndarray_ring.h"

#include <iostream>

using namespace mf;

TEST_CASE("ndarray_ring", "[ndarray_ring]") {
	ndsize<2> shape(321, 240);
	std::size_t duration = 10;
	ndarray_ring<2, int> ring(shape, duration);
	
	std::cout << "padding:" << ring.padding() << std::endl;
}