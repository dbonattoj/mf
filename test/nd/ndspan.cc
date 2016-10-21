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
#include "../../src/nd/ndspan.h"

using namespace mf;

TEST_CASE("ndspan", "[nd][ndspan]") {
	using span_type = ndspan<3, std::size_t>;
	
	span_type span(make_ndsize(1, 2, 3), make_ndsize(7, 6, 5));
	span_type span2(make_ndsize(1, 1, 1), make_ndsize(6, 6, 6));
	span_type span3(make_ndsize(1, 2, 3), make_ndsize(4, 4, 5));

	SECTION("basic") {
		span_type span(make_ndsize(1, 2, 3), make_ndsize(7, 6, 5));
		REQUIRE(span.start_pos() == make_ndsize(1, 2, 3));
		REQUIRE(span.end_pos() == make_ndsize(7, 6, 5));
		REQUIRE(span.shape() == make_ndsize(6, 4, 2));
		REQUIRE(span.size() == 6 * 4 * 2);
		
		REQUIRE(span == span);
		REQUIRE_FALSE(span != span);
		REQUIRE_FALSE(span == span2);
		REQUIRE(span != span2);

		span_type span_copy = span;
		REQUIRE(span_copy == span);
		REQUIRE_FALSE(span_copy != span);
		span_copy = span2;
		REQUIRE(span_copy == span2);
		REQUIRE_FALSE(span_copy != span2);
		
		REQUIRE(span.includes(make_ndsize(1, 2, 3)));
		REQUIRE_FALSE(span.includes(make_ndsize(7, 6, 5)));
		REQUIRE_FALSE(span.includes(make_ndsize(6, 5, 5)));
		REQUIRE(span.includes(make_ndsize(6, 5, 4)));
		
		REQUIRE(span.includes(make_ndsize(4, 4, 4)));
		REQUIRE_FALSE(span.includes(make_ndsize(8, 4, 4)));
		REQUIRE_FALSE(span.includes(make_ndsize(4, 4, 0)));
		REQUIRE_FALSE(span.includes(make_ndsize(7, 4, 4)));
		REQUIRE(span.includes(make_ndsize(4, 4, 3)));
	}
	
	SECTION("intersection") {
		// TODO
	}
	
	SECTION("iteration") {
		auto it = span3.begin();
		auto end = span3.end();
		REQUIRE(*(it++) == make_ndsize(1, 2, 3));
		REQUIRE(*(it++) == make_ndsize(1, 2, 4));
		REQUIRE(*(it++) == make_ndsize(1, 3, 3));
		REQUIRE(*(it++) == make_ndsize(1, 3, 4));
		REQUIRE(*(it++) == make_ndsize(2, 2, 3));
		REQUIRE(*(it++) == make_ndsize(2, 2, 4));
		REQUIRE(*(it++) == make_ndsize(2, 3, 3));
		REQUIRE(*(it++) == make_ndsize(2, 3, 4));
		REQUIRE(*(it++) == make_ndsize(3, 2, 3));
		REQUIRE(*(it++) == make_ndsize(3, 2, 4));
		REQUIRE(*(it++) == make_ndsize(3, 3, 3));
		REQUIRE(*(it++) == make_ndsize(3, 3, 4));		
		REQUIRE(it == end);
	}
}

