#include <catch.hpp>
#include "../src/ndcoord.h"

using namespace mf;

TEST_CASE("Construction ndcoord", "[ndcoord]") {
	SECTION("Construction, subscript, assignation, comparison") {
		ndcoord<3, int> a;
		REQUIRE(a[0] == 0);
		REQUIRE(a[1] == 0);
		REQUIRE(a[2] == 0);
		
		ndcoord<3, int> b(1, 2, 3);
		REQUIRE(b[0] == 1);
		REQUIRE(b[1] == 2);
		REQUIRE(b[2] == 3);

		ndcoord<3, int> c({4, 5, 6});
		REQUIRE(c[0] == 4);
		REQUIRE(c[1] == 5);
		REQUIRE(c[2] == 6);
		
		auto d = b;
		REQUIRE(d == b);
		
		d = a;
		REQUIRE(d == a);
		REQUIRE(!(d != a));
		
		REQUIRE(c != b);
		REQUIRE(!(c == b));
	}
	
	SECTION("Product") {
		REQUIRE(ndsize<3>(2, 5, 4).product() == 2 * 5 * 4);
		REQUIRE(ndsize<1>(3).product() == 3);
	}
}