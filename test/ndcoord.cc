#include <catch.hpp>
#include <iostream>
#include "../src/ndarray/ndcoord.h"

using namespace mf;

TEST_CASE("ndcoord", "[ndcoord]") {
	SECTION("construction") {
		ndcoord<3, int> zero;
		REQUIRE(zero[0] == 0);
		REQUIRE(zero[1] == 0);
		REQUIRE(zero[2] == 0);
		
		ndcoord<3, int> constant = 7;
		REQUIRE(constant[0] == 7);
		REQUIRE(constant[1] == 7);
		REQUIRE(constant[2] == 7);
				
		ndcoord<3, int> list{4, 5, 6};
		REQUIRE(list[0] == 4);
		REQUIRE(list[1] == 5);
		REQUIRE(list[2] == 6);
		
		ndcoord<3, int> copy = list;
		REQUIRE(copy[0] == 4);
		REQUIRE(copy[1] == 5);
		REQUIRE(copy[2] == 6);
		
		ndcoord<3, unsigned short> conv = copy;
		REQUIRE(conv[0] == 4);
		REQUIRE(conv[1] == 5);
		REQUIRE(conv[2] == 6);

		auto helper = make_ndcoord<int>(3, 5, 7);
		REQUIRE(helper[0] == 3);
		REQUIRE(helper[1] == 5);
		REQUIRE(helper[2] == 7);
		
		auto helper_size = make_ndsize(3, 5, 7);
		REQUIRE(helper_size[0] == 3);
		REQUIRE(helper_size[1] == 5);
		REQUIRE(helper_size[2] == 7);

		auto helper_ptrdiff = make_ndptrdiff(-3, 5, 7);
		REQUIRE(helper_ptrdiff[0] == -3);
		REQUIRE(helper_ptrdiff[1] == 5);
		REQUIRE(helper_ptrdiff[2] == 7);	
	}
	
	SECTION("basics") {
		ndcoord<3, int> coord{1, 2, 3};
		ndcoord<3, int> coord2{4, 5, 6};

		// subscript
		REQUIRE(coord[1] == 2);
		coord[1] = 4;
		REQUIRE(coord[1] == 4);
		REQUIRE(coord.front() == 1);
		REQUIRE(coord.back() == 3);
		
		// compare, assign
		REQUIRE_FALSE(coord == coord2);
		REQUIRE(coord != coord2);
	
		coord = coord2;
		REQUIRE(coord[0] == 4);
		REQUIRE(coord[1] == 5);
		REQUIRE(coord[2] == 6);
		REQUIRE(coord == coord2);
		REQUIRE_FALSE(coord != coord2);
		
		// product
		REQUIRE(coord.product() == 4*5*6);
		
		ndcoord<0, int> zero;
		REQUIRE(zero.product() == 1);
	}
	
	SECTION("section, cat") {
		auto c1 = make_ndsize(1, 2, 3, 4);
		
		REQUIRE(c1.head() == make_ndsize(1, 2, 3));
		REQUIRE(c1.tail() == make_ndsize(2, 3, 4));
		
		REQUIRE(c1.erase(1) == make_ndsize(1, 3, 4));
		REQUIRE(c1.erase(0) == c1.tail());
		REQUIRE(c1.erase(3) == c1.head());
	}
	
	SECTION("cat") {
		auto c1 = make_ndsize(1, 2, 3, 4);
		auto c2 = make_ndsize(5, 6);
		auto zero = make_ndsize();

		REQUIRE(ndcoord_cat(c1, c2) == make_ndsize(1, 2, 3, 4, 5, 6));
		REQUIRE(ndcoord_cat(c2, c1) == make_ndsize(5, 6, 1, 2, 3, 4));
		
		REQUIRE(ndcoord_cat(c1, zero) == c1);
		REQUIRE(ndcoord_cat(zero, c1) == c1);
		REQUIRE(ndcoord_cat(zero, zero) == zero);
	}
	
	SECTION("transform") {
		ndsize<3> c1{1, 2, 3}, c2{4, 5, 6};
		
		SECTION("transform_inplace") {
			c1.transform_inplace([](int i) { return i + 1; });
			REQUIRE(c1 == make_ndsize(2, 3, 4));
		
			c1.transform_inplace(c2, [](int a, int b) { return a * b; });
			REQUIRE(c1 == make_ndsize(2*4, 3*5, 4*6));
		}
		
		SECTION("plus") {
			REQUIRE(c1 + c2 == make_ndsize(1+4, 2+5, 3+6));
			c1 += c2;
			REQUIRE(c1 == make_ndsize(1+4, 2+5, 3+6));
		}
		
		SECTION("minus") {
			REQUIRE(c1 - c2 == make_ndsize(1-4, 2-5, 3-6));
			c1 -= c2;
			REQUIRE(c1 == make_ndsize(1-4, 2-5, 3-6));
		}

		SECTION("multiply") {
			REQUIRE(c1 * c2 == make_ndsize(1*4, 2*5, 3*6));
			c1 *= c2;
			REQUIRE(c1 == make_ndsize(1*4, 2*5, 3*6));
		}
		
		SECTION("divide") {
			REQUIRE(c2 / c1 == make_ndsize(4/1, 5/2, 6/3));
			c2 /= c1;
			REQUIRE(c2 == make_ndsize(4/1, 5/2, 6/3));
		}
		
		SECTION("unary") {
			REQUIRE(+c1 == c1);
			REQUIRE(-c1 == make_ndsize(-1, -2, -3));
		}
	}
	
	SECTION("1dim") {
		ndcoord<1, int> c = 2;
		c = 3;
		REQUIRE(c == 3);
		REQUIRE(3 == c);
	}
}