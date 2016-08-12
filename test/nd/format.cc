#include <catch.hpp>
#include <array>
#include <mf/ndarray/ndarray_view.h>
#include <mf/ndarray/ndarray_format.h>
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
	SECTION("data assignment") {
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
