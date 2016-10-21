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
#include "../../src/image/masked_image_view.h"
#include "../../src/nd/ndarray.h"
#include "../../src/color.h"

using namespace mf;


TEST_CASE("masked_image_view", "[image][masked_image_view]") {
	rgb_color c1(1, 2, 3), c2(4, 5, 6), c3(7, 8, 9), c4(10, 11, 12);

	auto shp = make_ndsize(10, 20);
	ndarray<2, rgb_color> arr(shp), arr2(shp);
	ndarray<2, byte> arrm(shp), arrm2(shp);
	for(std::ptrdiff_t i = 0; i < 10; ++i) for(std::ptrdiff_t j = 0; j < 20; ++j) {
		arr[i][j] = rgb_color(i, j, i+j);
		arrm[i][j] = i;
		arr2[i][j] = rgb_color(2*i, j, i+j);
		arrm2[i][j] = j;
	}
	

	SECTION("null") {
		masked_image_view<rgb_color> im;
		REQUIRE(im.is_null());
		REQUIRE(im.array_view().is_null());

		masked_image_view<const rgb_color> im_c;
		REQUIRE(im_c.is_null());
		REQUIRE(im.array_view().is_null());
	}
	
	SECTION("copy/move construct") {
		// construct masked_image_view(masked_image_view)
		masked_image_view<rgb_color> im(arr.view(), arrm.view());
		masked_image_view<rgb_color> im_(arr2.view(), arrm2.view());
		masked_image_view<rgb_color> im2(im);
		REQUIRE(same(im2.array_view(), im.array_view()));
		REQUIRE(same(im2.mask_array_view(), im.mask_array_view()));
		
		// construct const masked_image_view(masked_image_view)
		masked_image_view<const rgb_color> im3(im);
		REQUIRE(same(im3.array_view(), im.array_view()));
		REQUIRE(same(im3.mask_array_view(), im.mask_array_view()));
		
		// construct const masked_image_view(const masked_image_view)
		masked_image_view<const rgb_color> im4(im3);
		
		// move-construct masked_image_view(masked_image_view)
		masked_image_view<rgb_color> im5(std::move(im));
		REQUIRE(same(im5.array_view(), im.array_view()));
		REQUIRE(same(im5.mask_array_view(), im.mask_array_view()));

		// masked_image_view.reset(masked_image_view)
		im5.reset(im2);
		REQUIRE(same(im5.array_view(), im2.array_view()));
		REQUIRE(same(im5.mask_array_view(), im2.mask_array_view()));

		// const masked_image_view.reset(masked_image_view)
		im3.reset(im_);
		REQUIRE(same(im3.array_view(), im_.array_view()));
		REQUIRE(same(im3.mask_array_view(), im_.mask_array_view()));

		// const masked_image_view.reset(const masked_image_view)
		im3.reset(im4);
		REQUIRE(same(im3.array_view(), im4.array_view()));
		REQUIRE(same(im3.mask_array_view(), im4.mask_array_view()));
	}
	
	SECTION("from ndarray_view") {
		// construct masked_image_view(ndarray_view)
		ndarray_view<2, rgb_color> ndvw = arr.view();
		ndarray_view<2, byte> ndvwm = arrm.view();
		masked_image_view<rgb_color> im(ndvw, ndvwm);
		REQUIRE_FALSE(im.is_null());
		REQUIRE(same(im.array_view(), ndvw));
		REQUIRE(same(im.mask_array_view(), ndvwm));
		REQUIRE(im.shape() == ndvw.shape());
		
		// construct masked_image_view(invalid ndarray_view, invalid ndarray_view)
		ndarray_view<2, rgb_color> ndvw3 = ndvw.section(ndvw.full_span(), make_ndptrdiff(2, 2));
		ndarray_view<2, byte> ndvw3m = ndvwm.section(ndvwm.full_span(), make_ndptrdiff(2, 2));
		REQUIRE_FALSE(ndvw3.has_default_strides_without_padding());
		REQUIRE_THROWS(new masked_image_view<rgb_color>(ndvw3, ndvw3m));
		
		// construct masked_image_view(ndarray_view, diff shaped ndarray_view)
		ndarray_view<2, byte> ndvwm_ = arrm.section(make_ndsize(0, 0), make_ndsize(5, 5));
		REQUIRE_THROWS(new masked_image_view<rgb_color>(ndvw, ndvwm_));
	}
	
	SECTION("from const ndarray_view") {
		// construct const image_view(const ndarray_view, const ndarray_view)
		ndarray_view<2, const rgb_color> ndvw = arr.view();
		ndarray_view<2, const byte> ndvwm = arrm.view();
		masked_image_view<const rgb_color> im(ndvw, ndvwm);
		REQUIRE(same(im.array_view(), ndvw));
		REQUIRE(same(im.mask_array_view(), ndvwm));
		
		// construct const image_view(ndarray_view, ndarray_view)
		ndarray_view<2, rgb_color> ndvw2 = arr.view();
		ndarray_view<2, byte> ndvwm2 = arrm.view();
		masked_image_view<const rgb_color> im2(ndvw2, ndvwm2);
		REQUIRE(same(im2.array_view(), ndvw2));
		REQUIRE(same(im2.mask_array_view(), ndvwm2));

		// construct const image_view(const ndarray_view, ndarray_view)
		masked_image_view<const rgb_color> im3(ndvw, ndvwm2);
		REQUIRE(same(im3.array_view(), ndvw));
		REQUIRE(same(im3.mask_array_view(), ndvwm2));

		// construct const image_view(ndarray_view, ndarray_view)
		masked_image_view<const rgb_color> im4(ndvw2, ndvwm);
		REQUIRE(same(im4.array_view(), ndvw2));
		REQUIRE(same(im4.mask_array_view(), ndvwm));
	}
	
	SECTION("from opencv mat") {
		masked_image_view<rgb_color> im(arr.view(), arrm.view());
		auto cvmat = im.cv_mat();
		auto cvmatm = im.cv_mask_mat();

		// construct image_view(cv_mat)
		masked_image_view<rgb_color> im2(cvmat, cvmatm);
		// cvmat and im must reference same memory
		REQUIRE(im.cv_mat()(3, 4) == cvmat(3, 4));
		REQUIRE(im.cv_mask_mat()(3, 4) == cvmatm(3, 4));
		cvmat(3, 4) = c1;
		cvmatm(3, 4) = 123;
		REQUIRE(im.cv_mat()(3, 4) == c1);
		REQUIRE(im.cv_mask_mat()(3, 4) == 123);
		im.cv_mat()(5, 8) = c2;
		im.cv_mask_mat()(5, 8) = 111;
		REQUIRE(cvmat(5, 8) == c2);
		REQUIRE(cvmatm(5, 8) == 111);
		
		// construct const image_view(cv_mat)
		masked_image_view<const rgb_color> im3(cvmat, cvmatm);
	}
}
