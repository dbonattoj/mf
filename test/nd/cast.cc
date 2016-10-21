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
#include "../../src/nd/ndarray_view.h"
#include "../../src/nd/ndarray_view_cast.h"
#include "../support/ndarray.h"
#include <cstdint>

using namespace mf;
using namespace mf::test;


TEST_CASE("ndarray_view_cast", "[nd][ndarray_view_cast]") {
	constexpr std::size_t len = 3 * 4 * 4;
	auto shp = make_ndsize(3, 4, 4);


	SECTION("elem from tuple") {
		using tuple_type = elem_tuple<float, int>;
		using view_type = ndarray_view<3, tuple_type>;
	
		std::vector<tuple_type> raw(len);
		for(int i = 0; i < len; ++i) raw.push_back(tuple_type(2.0f * i, i));

		view_type arr(raw.data(), shp);
		
		using float_view_type = ndarray_view<3, float>;
		using int_view_type = ndarray_view<3, int>;

		float_view_type float_arr = ndarray_view_cast<float_view_type>(arr);
		int_view_type int_arr = ndarray_view_cast<int_view_type>(arr);

		for(auto it = arr.begin(); it != arr.end(); ++it) {
			auto coord = it.coordinates();
			const tuple_type& tup = *it;
			float& f = float_arr.at(coord);
			int& i = int_arr.at(coord);
			REQUIRE(get<float>(tup) == f);
			REQUIRE(get<int>(tup) == i);
		}
	}
	
	SECTION("scalar from vec-elem") {
		using elem_type = std::array<int, 5>;
		using view_type = ndarray_view<3, elem_type>;

		std::vector<elem_type> raw(len);
		for(int i = 0; i < len; ++i) raw.push_back({i, 2*i, 3*i, 5*i, 8*i});
		
		view_type arr(raw.data(), shp);
		
		using scalar_view_type = ndarray_view<4, int>;
		
		scalar_view_type scalar_arr = ndarray_view_cast<scalar_view_type>(arr);
		REQUIRE(scalar_arr.shape() == make_ndsize(3, 4, 4, 5));
		
		for(auto it = arr.begin(); it != arr.end(); ++it) {
			auto coord = it.coordinates();
			const elem_type& elem = *it;
		
			std::ptrdiff_t x = coord[0], y = coord[1], z = coord[2];
			
			REQUIRE(scalar_arr[x][y][z][0] == elem[0]);
			REQUIRE(scalar_arr[x][y][z][1] == elem[1]);
			REQUIRE(scalar_arr[x][y][z][2] == elem[2]);
			REQUIRE(scalar_arr[x][y][z][3] == elem[3]);
			REQUIRE(scalar_arr[x][y][z][4] == elem[4]);
		}
	}
	
	
	SECTION("reinterpret") {
		struct three { std::uint8_t a, b, c; };

		using int32_view_type = ndarray_view<3, std::int32_t>;
		using uint16_view_type = ndarray_view<3, std::uint16_t>;
		using three_view_type = ndarray_view<3, three>;
		
		REQUIRE(alignof(std::int32_t) == 4);
		REQUIRE(sizeof(std::int32_t) == 4);
				
		REQUIRE(alignof(std::uint16_t) == 2);
		REQUIRE(sizeof(std::uint16_t) == 2);

		REQUIRE(alignof(three) == 1);
		REQUIRE(sizeof(three) == 3);

		SECTION("int32 -> uint16") {
			std::vector<std::int32_t> raw(len);
			for(int i = 0; i < len; ++i) raw.push_back(i);

			int32_view_type int32_view(raw.data(), shp);
			uint16_view_type uint16_view = ndarray_view_reinterpret_cast<uint16_view_type>(int32_view);
					
			REQUIRE(uint16_view.shape() == int32_view.shape());
			
			// modify every second element in uint16_view
			for(auto it = uint16_view.begin(); it < uint16_view.end(); it += 2) *it = 123;
			
			// others must have remained unchanged in int32_view
			for(auto it = int32_view.begin() + 1; it < int32_view.end(); it += 2)
				REQUIRE(*it == raw[it.index()]);
		}
		
		SECTION("uint16 -> int32 (padded)") {
			// uint16_t array, with 2 byte padding
			std::vector<std::uint16_t> raw(len);
			for(int i = 0; i < len*2; ++i) { raw.push_back(0); raw.push_back(i); }
			uint16_view_type uint16_view(raw.data(), shp, uint16_view_type::default_strides(shp, 2));
	
			int32_view_type int32_view = ndarray_view_reinterpret_cast<int32_view_type>(uint16_view);
					
			REQUIRE(uint16_view.shape() == int32_view.shape());
			
			// modify every second element in int32_view
			for(auto it = int32_view.begin(); it < int32_view.end(); it += 2) *it = 123;
			
			// others must have remained unchanged in uint16_view
			for(auto it = uint16_view.begin() + 1; it < uint16_view.end(); it += 2)
				REQUIRE(*it == raw[it.index()]);
		}

		SECTION("alignment ok, but too small") {	
			std::vector<std::uint16_t> raw(len);
			uint16_view_type uint16_view(raw.data(), shp);
			REQUIRE_THROWS(ndarray_view_reinterpret_cast<three_view_type>(uint16_view));
		}

		SECTION("size ok, but incompatible alignment") {
			std::vector<three> raw(len);
			three_view_type three_view(raw.data(), shp);
			REQUIRE_THROWS(ndarray_view_reinterpret_cast<int32_view_type>(three_view));
		}
		
		SECTION("size ok, padded for alignment") {
			std::vector<three> raw(len);
			three_view_type three_view(raw.data(), shp, three_view_type::default_strides(shp, 1));
			ndarray_view_reinterpret_cast<int32_view_type>(three_view);
		}
		
		SECTION("size ok, padded for alignment 2") {
			std::vector<three> raw(len);
			three_view_type three_view(raw.data(), shp, three_view_type::default_strides(shp, 1+4));
			ndarray_view_reinterpret_cast<int32_view_type>(three_view);
		}
	}
}
