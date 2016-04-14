/*
#include <catch.hpp>
#include <type_traits>
#include "../src/elem_tuple.h"
#include "../src/utility/misc.h"

using namespace mf;

TEST_CASE("elem_tuple", "[elem_tuple]") {
	using tuple_type = elem_tuple<float, int, char>;
	tuple_type tup(1.0f, 2, 'a');
		
	const tuple_type& ctup = tup;
	
	static_assert(std::is_pod<tuple_type>::value, "elem_tuple not POD");

	SECTION("get") {		
		static_assert(std::is_same<decltype(get<0>(tup)), float&>::value, "elem_tuple get fail");
		static_assert(std::is_same<decltype(get<1>(tup)), int&>::value, "elem_tuple get fail");
		static_assert(std::is_same<decltype(get<2>(tup)), char&>::value, "elem_tuple get fail");
		REQUIRE(get<0>(tup) == 1.0f);
		REQUIRE(get<1>(tup) == 2);
		REQUIRE(get<2>(tup) == 'a');
		
		static_assert(std::is_same<decltype(get<0>(ctup)), const float&>::value, "const elem_tuple get fail");
		static_assert(std::is_same<decltype(get<1>(ctup)), const int&>::value, "const elem_tuple get fail");
		static_assert(std::is_same<decltype(get<2>(ctup)), const char&>::value, "const elem_tuple get fail");		
		REQUIRE(get<0>(ctup) == 1.0f);
		REQUIRE(get<1>(ctup) == 2);
		REQUIRE(get<2>(ctup) == 'a');
	}
	
	SECTION("get by type") {
		static_assert(std::is_same<decltype(get<float>(tup)), float&>::value, "elem_tuple get fail");
		static_assert(std::is_same<decltype(get<int>(tup)), int&>::value, "elem_tuple get fail");
		static_assert(std::is_same<decltype(get<char>(tup)), char&>::value, "elem_tuple get fail");
		REQUIRE(get<float>(tup) == 1.0f);
		REQUIRE(get<int>(tup) == 2);
		REQUIRE(get<char>(tup) == 'a');
		
		static_assert(std::is_same<decltype(get<float>(ctup)), const float&>::value, "const elem_tuple get fail");
		static_assert(std::is_same<decltype(get<int>(ctup)), const int&>::value, "const elem_tuple get fail");
		static_assert(std::is_same<decltype(get<char>(ctup)), const char&>::value, "const elem_tuple get fail");		
		REQUIRE(get<float>(ctup) == 1.0f);
		REQUIRE(get<int>(ctup) == 2);
		REQUIRE(get<char>(ctup) == 'a');
	}
	
	SECTION("index by type") {
		REQUIRE( (elem_tuple_index<float, tuple_type>) == 0);
		REQUIRE( (elem_tuple_index<int, tuple_type>) == 1);
		REQUIRE( (elem_tuple_index<char, tuple_type>) == 2);
	}
	
	SECTION("get and set") {
		get<0>(tup) = 123.0f;
		get<char>(tup) = 'b';
		REQUIRE(get<0>(tup) == 123.0f);
		REQUIRE(get<1>(tup) == 2);
		REQUIRE(get<2>(tup) == 'b');
	}
	
	SECTION("offset") {
		REQUIRE( (elem_tuple_offset<0, tuple_type>) == 0);
		REQUIRE( (elem_tuple_offset<1, tuple_type>) == sizeof(float));
		REQUIRE( (elem_tuple_offset<2, tuple_type>) == sizeof(float) + sizeof(int));
		
		*reinterpret_cast<float*>(advance_raw_ptr(&tup, elem_tuple_offset<0, tuple_type>)) = 123.0f;
		*reinterpret_cast<int*>(advance_raw_ptr(&tup, elem_tuple_offset<1, tuple_type>)) = 456;
		*reinterpret_cast<char*>(advance_raw_ptr(&tup, elem_tuple_offset<2, tuple_type>)) = 'b';
		
		REQUIRE(get<0>(tup) == 123.0f);
		REQUIRE(get<1>(tup) == 456);
		REQUIRE(get<2>(tup) == 'b');
	}
	
	SECTION("assign") {
		tuple_type tup2(123.0f, 456, 'b');
		tup = tup2;
		REQUIRE(get<0>(tup) == 123.0f);
		REQUIRE(get<1>(tup) == 456);
		REQUIRE(get<2>(tup) == 'b');		
	}
	
	SECTION("compare") {
		tuple_type tup2(123.0f, 456, 'b');
		REQUIRE(tup == tup);
		REQUIRE_FALSE(tup != tup);

		tuple_type tup_equal(1.0f, 2, 'a');
		REQUIRE(tup == tup_equal);
		REQUIRE_FALSE(tup != tup_equal);

		tuple_type tup_diff(1.0f, 2, 'b');
		REQUIRE_FALSE(tup == tup_diff);
		REQUIRE(tup != tup_diff);
	}
}
*/
