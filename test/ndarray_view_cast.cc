#include <catch.hpp>
#include <array>
#include "../src/ndarray/ndarray_view.h"
#include "../src/ndarray/ndarray_view_cast.h"
#include "support/ndarray.h"

using namespace mf;
using namespace mf::test;


TEST_CASE("ndarray_view cast", "[ndarray_view][elem_tuple]") {
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
}
