/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <catch.hpp>
#include "../src/ndarray/ndarray.h"
#include "support/ndarray.h"

using namespace mf;
using namespace mf::test;

TEST_CASE("ndarray", "[ndarray]") {
	constexpr std::ptrdiff_t l = sizeof(int);
	ndsize<3> shape{3, 4, 4};
	std::vector<int> raw(shape.product());
	for(int i = 0; i < raw.size(); ++i) raw[i] = i;
	ndarray_view<3, int> arr_vw(raw.data(), shape);
	using array_type = ndarray<3, int>;


	SECTION("construction") {
		// default construction
		array_type arr(shape);
		REQUIRE(arr.shape() == shape);
		arr[1][2][3] = 123;
		REQUIRE(arr[1][2][3] == 123);
		
		// copy construction from view
		array_type arr2(arr_vw);
		REQUIRE(arr2[1][2][3] == arr_vw[1][2][3]);
		REQUIRE(arr2.shape() == shape);
		arr2[1][2][3] = 456;
		REQUIRE(arr2[1][2][3] == 456);
		REQUIRE_FALSE(arr_vw[1][2][3] == 456);
	}
	
	
	SECTION("assignment") {
		array_type arr(shape);
		arr[1][2][3] = 0;
		array_type arr2(arr_vw);
		arr2[1][2][3] = 456;
	
		// assignment (ndarray)
		arr = arr2;
		REQUIRE(arr[1][2][3] == 456);
		arr[1][2][3] = 789;
		REQUIRE(arr[1][2][3] == 789);
		REQUIRE_FALSE(arr2[1][2][3] == 789);
		
		// assignment (ndarray_view);
		arr = arr_vw;
		REQUIRE(arr[1][2][3] == arr_vw[1][2][3]);
		arr[1][2][3] = 7890;
		REQUIRE(arr[1][2][3] == 7890);
		REQUIRE_FALSE(arr_vw[1][2][3] == 7890);
		
		// assignment (ndarray, different shape)
		ndsize<3> shape_{2, 2, 5};
		array_type arr3(shape_);
		arr3[1][1][1] = 23;
		REQUIRE(arr.shape() == shape);
		arr = arr3;
		REQUIRE(arr.shape() == shape_);
		REQUIRE(arr[1][1][1] == 23);
		arr[1][1][1] = 45;
		REQUIRE(arr[1][1][1] == 45);
		REQUIRE_FALSE(arr3[1][1][1] == 45);
	}
	
	SECTION("make") {
		// make from arr_vw (same as copy-construction)
		auto arr = make_ndarray(arr_vw);
		REQUIRE(arr[1][2][3] == arr_vw[1][2][3]);
		arr[1][2][3] = 7890;
		REQUIRE(arr[1][2][3] == 7890);
		REQUIRE_FALSE(arr_vw[1][2][3] == 7890);
	
		// make from slice of arr_vw (--> different dimension)
		auto arr2 = make_ndarray(arr_vw[1]);
		REQUIRE(arr2[2][3] == arr_vw[1][2][3]);
		REQUIRE(arr2.shape() == arr_vw[1].shape());
		arr2[2][3] = 1230;
		REQUIRE(arr2[2][3] == 1230);
		REQUIRE_FALSE(arr_vw[1][2][3] == 1230);
	}
}
