#include <catch.hpp>
#include <mf/image/image.h>
#include <mf/nd/ndarray.h>
#include <mf/color.h>

using namespace mf;

TEST_CASE("image_view", "[image_view]") {
	auto shp = make_ndsize(10, 20);
	ndarray<2, rgb_color> arr(shp), arr2(shp);
	for(std::ptrdiff_t i = 0; i < 10; ++i) for(std::ptrdiff_t j = 0; j < 20; ++j) {
		arr[i][j] = rgb_color(i, j, i+j);
		arr2[i][j] = rgb_color(2*i, j, i+j);
	}
	
	rgb_color c1(1, 2, 3);
	rgb_color c2(4, 5, 6);
	rgb_color c3(7, 8, 9);
	rgb_color c4(10, 11, 12);

	SECTION("null") {
		image_view<rgb_color> im;
		REQUIRE(im.is_null());

		image_view<const rgb_color> im_c;
		REQUIRE(im_c.is_null());
	}

	SECTION("from ndarray_view") {
		ndarray_view<2, rgb_color> ndvw = arr.view();
		image_view<rgb_color> im(ndvw);
		
		// ndvw and im must reference same memory
		REQUIRE(im.cv_mat()(3, 4) == ndvw[3][4]);
		ndvw[3][4] = c1;
		REQUIRE(im.cv_mat()(3, 4) == c1);
		im.cv_mat()(5, 8) = c2;
		REQUIRE(ndvw[5][8] == c2);

		ndarray_view<2, rgb_color> ndvw2 = arr2.view();	
		image_view<rgb_color> im2(ndvw2);
		im.reset(im2);
		// npw ndvw2 and im must reference same memory
		REQUIRE(im.cv_mat()(3, 4) == ndvw2[3][4]);
		ndvw2[3][4] = c3;
		REQUIRE(im.cv_mat()(3, 4) == c3);
		im.cv_mat()(5, 8) = c4;
		REQUIRE(ndvw2[5][8] == c4);
		REQUIRE_FALSE(ndvw[5][8] == c4);
	}
}
