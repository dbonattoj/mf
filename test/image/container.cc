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
#include "../../src/image/image.h"
#include "../../src/image/image_view.h"
#include "../../src/nd/ndarray.h"
#include "../../src/color.h"

using namespace mf;


TEST_CASE("image", "[image]") {
	rgb_color c1(1, 2, 3), c2(4, 5, 6), c3(7, 8, 9), c4(10, 11, 12);

	auto shp = make_ndsize(10, 20);
	ndarray<2, rgb_color> arr(shp), arr2(shp);
	for(std::ptrdiff_t i = 0; i < 10; ++i) for(std::ptrdiff_t j = 0; j < 20; ++j) {
		arr[i][j] = rgb_color(i, j, i+j);
		arr2[i][j] = rgb_color(2*i, j, i+j);
	}
	
	SECTION("from shape") {
		// image(shape)
		image<rgb_color> im(shp);
		REQUIRE(im.shape() == shp);
		REQUIRE_FALSE(im.view().is_null());
		REQUIRE(im.view().shape() == shp);
		REQUIRE_FALSE(im.array_view().is_null());
		REQUIRE(im.array_view().shape() == shp);
	}

	SECTION("from ndarray_view") {
		// image(ndarray_view)
		image<rgb_color> im(arr.view());
		REQUIRE_FALSE(same(im.array_view(), arr.view()));
		REQUIRE(im.array_view() == arr.view());
		REQUIRE(im.shape() == arr.shape());
	}
	
	SECTION("from image_view") {
		// image(image_view)
		image_view<rgb_color> imvw(arr.view());
		image_view<rgb_color> imvw2(arr2.view());
		image<rgb_color> im(imvw);
		REQUIRE_FALSE(same(im.array_view(), arr.view()));
		REQUIRE(im.array_view() == arr.view());
		REQUIRE(im.shape() == arr.shape());
		
		// image = image_view
		image<rgb_color> im2(shp);
		im2 = imvw2;
		REQUIRE_FALSE(same(im.array_view(), arr.view()));
		REQUIRE(im.array_view() == arr.view());
		REQUIRE(im2.shape() == arr.shape());
	}
	
	SECTION("from const image_view") {
		// image(const image_view)
		image_view<const rgb_color> imvw(arr.view());
		image_view<const rgb_color> imvw2(arr2.view());
		image<rgb_color> im(imvw);
		REQUIRE(im.array_view() == arr.view());
		
		// image = const image_view
		image<rgb_color> im2(shp);
		im2 = imvw2;
		REQUIRE(im.array_view() == arr.view());
	}

	SECTION("from opencv mat") {
		// image(opencv_mat)
		image_view<rgb_color> imvw(arr.view());
		auto cvmat = imvw.cv_mat();
		
		image<rgb_color> im(cvmat);
		REQUIRE(im.array_view() == imvw.array_view());
	}
	
	SECTION("copy construct/assign") {
		// image(image)
		image<rgb_color> im(arr.view());
		image<rgb_color> im2(im);
		REQUIRE_FALSE(same(im.array_view(), im2.array_view()));
		REQUIRE(im.array_view() == im2.array_view());
		
		// image = image
		image<rgb_color> im3(arr.view());
		im3 = im2;
		REQUIRE_FALSE(same(im3.array_view(), im2.array_view()));
		REQUIRE(im3.array_view() == im2.array_view());
	}
	
	SECTION("move construct/assign") {
		// image(image&&)
		image<rgb_color> im(arr.view());
		image<rgb_color> im2(std::move(im));
		REQUIRE(same(im.array_view(), im2.array_view()));
		
		// image = image&&
		image<rgb_color> im3(arr.view());
		im3 = std::move(im2);
		REQUIRE(same(im3.array_view(), im2.array_view()));
	}
}
