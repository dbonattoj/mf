#include <catch.hpp>
#include <mf/image/image.h>
#include <mf/image/image_view.h>
#include <mf/nd/ndarray.h>
#include <mf/color.h>

using namespace mf;


TEST_CASE("image (container)", "[image]") {
	rgb_color c1(1, 2, 3), c2(4, 5, 6), c3(7, 8, 9), c4(10, 11, 12);

	auto shp = make_ndsize(10, 20);
	ndarray<2, rgb_color> arr(shp), arr2(shp);
	for(std::ptrdiff_t i = 0; i < 10; ++i) for(std::ptrdiff_t j = 0; j < 20; ++j) {
		arr[i][j] = rgb_color(i, j, i+j);
		arr2[i][j] = rgb_color(2*i, j, i+j);
	}
	
	SECTION("from shape") {
		image<rgb_color> im(shp);
		REQUIRE(im.shape() == shp);
		REQUIRE_FALSE(im.view().is_null());
		REQUIRE(im.view().shape() == shp);
		REQUIRE_FALSE(im.array_view().is_null());
		REQUIRE(im.array_view().shape() == shp);
	}
	

	SECTION("from ndarray_view") {
		image<rgb_color> im(arr.view());
		REQUIRE_FALSE(same(im.array_view(), arr.view()));
		REQUIRE(im.array_view() == arr.view());
		REQUIRE(im.shape() == arr.shape());
		
		image<rgb_color> im2(shp);
		im2 = arr.view();
		REQUIRE_FALSE(same(im.array_view(), arr.view()));
		REQUIRE(im.array_view() == arr.view());
		REQUIRE(im2.shape() == arr.shape());
	}
	
	SECTION("from image_view") {
		image_view<rgb_color> imvw(arr.view());
		image_view<rgb_color> imvw2(arr2.view());
		image<rgb_color> im(imvw);
		REQUIRE_FALSE(same(im.array_view(), arr.view()));
		REQUIRE(im.array_view() == arr.view());
		REQUIRE(im.shape() == arr.shape());
		
		image<rgb_color> im2(shp);
		im2 = imvw2;
		REQUIRE_FALSE(same(im.array_view(), arr.view()));
		REQUIRE(im.array_view() == arr.view());
		REQUIRE(im2.shape() == arr.shape());
	}
	
	SECTION("from const image_view") {
		image_view<const rgb_color> imvw(arr.view());
		image_view<const rgb_color> imvw2(arr2.view());
		image<rgb_color> im(imvw);
		REQUIRE(im.array_view() == arr.view());
		
		image<rgb_color> im2(shp);
		im2 = imvw2;
		REQUIRE(im.array_view() == arr.view());
	}

	SECTION("from opencv mat") {
		image_view<rgb_color> imvw(arr.view());
		auto cvmat = imvw.cv_mat();
		
		image<rgb_color> im(cvmat);
		REQUIRE(im.array_view() == imvw.array_view());
	}
}
