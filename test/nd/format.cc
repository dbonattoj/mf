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
#include <array>
#include <mf/nd/ndarray_view.h>
#include <mf/nd/ndarray_format.h>
#include "../support/ndarray.h"

using namespace mf;
using namespace mf::test;

constexpr std::size_t len = 3 * 4 * 4;
ndsize<3> shp{3, 4, 4};


template<std::size_t Elem_size, std::size_t Padding_size>
void test_non_contiguous_() {
	struct elem_t {
		std::array<byte, Elem_size> data;
		bool operator==(const elem_t& other) const
			{ return (data == other.data); }
	};
	struct padded_elem_t {
		elem_t elem;
		std::array<byte, Padding_size> padding;
		bool operator==(const padded_elem_t& other) const
			{ return (elem.data == other.elem.data); }
	};
	REQUIRE(sizeof(elem_t) == Elem_size);
	REQUIRE(sizeof(padded_elem_t) == Elem_size + Padding_size);
	ndarray_format frm = make_ndarray_format<elem_t>(len, Elem_size + Padding_size);
	REQUIRE_FALSE(frm.is_contiguous());
	
	std::vector<padded_elem_t> raw1(len), raw2(len);
	for(int i = 0; i < len; ++i) {
		for(int j = 0; j < Elem_size; ++j) {
			raw1[i].elem.data[j] = i + j;
			raw2[i].elem.data[j] = 2*i + 1 + j;
		}
		raw1[i].padding.fill(123);
		raw2[i].padding.fill(231);
	}
	
	REQUIRE(ndarray_data_compare(
		static_cast<const void*>(raw1.data()),
		static_cast<const void*>(raw1.data()),
		frm
	));
	REQUIRE_FALSE(ndarray_data_compare(
		static_cast<const void*>(raw1.data()),
		static_cast<const void*>(raw2.data()),
		frm
	));
	ndarray_data_copy(
		static_cast<void*>(&raw2[0]),
		static_cast<const void*>(raw1.data()),
		frm
	);
	REQUIRE(raw1 == raw2);
	for(int i = 0; i < len; ++i) {
		std::array<byte, Padding_size> expected_pad; expected_pad.fill(123);
		REQUIRE(raw1[i].padding == expected_pad);
	}
	REQUIRE(ndarray_data_compare(
		static_cast<const void*>(raw1.data()),
		static_cast<const void*>(raw2.data()),
		frm
	));
	raw1[2].padding[0] = 77;
	raw2[5].padding[0] = 99;
	REQUIRE(ndarray_data_compare(
		static_cast<const void*>(raw1.data()),
		static_cast<const void*>(raw2.data()),
		frm
	));
}

TEST_CASE("ndarray_format", "[nd][ndarray_format]") {	
	SECTION("data assignment and comparison") {
		SECTION("contiguous") {
			std::vector<int> raw1(len), raw2(len);
			for(int i = 0; i < len; ++i) {
				raw1[i] = i;
				raw2[i] = 2*i + 1;
			}
			ndarray_format frm = make_ndarray_format<int>(len);
			REQUIRE(frm.is_contiguous());
			
			REQUIRE(ndarray_data_compare(
				static_cast<const void*>(raw1.data()),
				static_cast<const void*>(raw1.data()),
				frm
			));
			REQUIRE_FALSE(ndarray_data_compare(
				static_cast<const void*>(raw1.data()),
				static_cast<const void*>(raw2.data()),
				frm
			));
			ndarray_data_copy(
				static_cast<void*>(&raw2[0]),
				static_cast<const void*>(raw1.data()),
				frm
			);
			REQUIRE(raw1 == raw2);
			REQUIRE(ndarray_data_compare(
				static_cast<const void*>(raw1.data()),
				static_cast<const void*>(raw2.data()),
				frm
			));
		}
		

		SECTION("non-contiguous") {
			SECTION("int8 boundary") { test_non_contiguous_<1, 5>(); }
			SECTION("int16 boundary") { test_non_contiguous_<2, 8>(); }
			SECTION("int32 boundary") { test_non_contiguous_<4, 4>(); }
			SECTION("int64 boundary") { test_non_contiguous_<8, 16>(); }
			SECTION("irregular") { test_non_contiguous_<105, 13>(); }
			SECTION("not int32 boundary") { test_non_contiguous_<4, 5>(); }
		}
	}
}
