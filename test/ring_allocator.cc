#include <catch.hpp>
#include <algorithm>
#include "../src/ring_allocator.h"
#include "../src/util/memory.h"

using namespace mf;

TEST_CASE("ring_allocator", "[ring_allocator]") {
	ring_allocator<int> allocator;
	std::size_t n = round_up_to_fit_system_page_size<int>(100);
	
	// allocate
	int* buf = allocator.allocate(n);
	REQUIRE(buf != nullptr);
	
	// write in base, read in mirror
	buf[7] = 123;
	REQUIRE(buf[7] == 123);
	REQUIRE(buf[n + 7] == 123);
	
	// write in mirror, read in base
	buf[n + 3] = 456;
	REQUIRE(buf[n + 3] == 456);
	REQUIRE(buf[3] == 456);
	
	// write segment, warp-around
	std::vector<int> data;
	for(int i = 0; i < 100; ++i) data.push_back(i);
	std::ptrdiff_t beg = n - 50;
	std::copy(data.cbegin(), data.cend(), buf + beg);
	REQUIRE(std::equal(data.cbegin(), data.cend(), buf + beg));
	REQUIRE(std::equal(data.cbegin() + 50, data.cend(), buf));
	
	// deallocate
	allocator.deallocate(buf, n);
}