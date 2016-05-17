#include <catch.hpp>
#include <algorithm>
#include "../src/os/memory.h"
#include "../src/common.h"

using namespace mf;


TEST_CASE("ring_allocator", "[ring_allocator]") {
	raw_ring_allocator allocator;
	std::size_t n = raw_round_up_to_fit_system_page_size(100);
	
	// allocate
	void* buf_raw = allocator.raw_allocate(n);
	REQUIRE(buf_raw != nullptr);
	byte* buf = static_cast<byte*>(buf_raw);
	
	// write in base, read in mirror
	buf[7] = byte(123);
	REQUIRE(buf[7] == byte(123));
	REQUIRE(buf[n + 7] == byte(123));
	
	// write in mirror, read in base
	buf[n + 3] = byte(456);
	REQUIRE(buf[n + 3] == byte(456));
	REQUIRE(buf[3] == byte(456));
	
	// write segment, warp-around
	std::vector<byte> data;
	for(byte i = 0; i < byte(100); ++i) data.push_back(i);
	std::ptrdiff_t beg = n - 50;
	std::copy(data.cbegin(), data.cend(), buf + beg);
	REQUIRE(std::equal(data.cbegin(), data.cend(), buf + beg));
	REQUIRE(std::equal(data.cbegin() + 50, data.cend(), buf));
	
	// deallocate
	allocator.raw_deallocate(buf_raw, n);
}
