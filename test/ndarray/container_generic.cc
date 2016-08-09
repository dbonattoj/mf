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
#include <mf/ndarray/generic/ndarray_view_generic.h>
#include <mf/ndarray/generic/ndarray_timed_view_generic.h>
#include <mf/ndarray/generic/ndarray_generic.h>

using namespace mf;
/*
TEST_CASE("ndarray_generic", "[nd][ndarray_generic][ndarray_view_generic][frame_format]") {
	SECTION("null") {
		ndarray_generic<2> null_arr;
		REQUIRE(null_arr.is_null());
		REQUIRE_FALSE(null_arr.format().is_defined());
	}
	
	SECTION("one array format") {
		std::size_t n = 100;
		auto gen_shp = make_ndsize(3, 4);
		frame_format frm = make_frame_array_format<int>(n);
		
		ndarray_generic<2> arr(frm, gen_shp);
		REQUIRE(arr.format() == frm);
		
		ndarray_view_generic<1> vw = arr[0];
		REQUIRE(vw.format() == frm);
	}
}

*/
