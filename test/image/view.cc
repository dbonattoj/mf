#include <catch.hpp>
#include <mf/image/image_view.h>
#include <mf/nd/ndarray.h>
#include <mf/color.h>

using namespace mf;


TEST_CASE("image_view", "[image][image_view]") {
	rgb_color c1(1, 2, 3), c2(4, 5, 6), c3(7, 8, 9), c4(10, 11, 12);

	auto shp = make_ndsize(10, 20);
	ndarray<2, rgb_color> arr(shp), arr2(shp);
	for(std::ptrdiff_t i = 0; i < 10; ++i) for(std::ptrdiff_t j = 0; j < 20; ++j) {
		arr[i][j] = rgb_color(i, j, i+j);
		arr2[i][j] = rgb_color(2*i, j, i+j);
	}
	
	SECTION("null") {
		image_view<rgb_color> im;
		REQUIRE(im.is_null());
		REQUIRE(im.array_view().is_null());

		image_view<const rgb_color> im_c;
		REQUIRE(im_c.is_null());
		REQUIRE(im.array_view().is_null());
	}
	
	SECTION("copy/move construct") {
		// construct image_view(image_view)
		image_view<rgb_color> im(arr.view());
		image_view<rgb_color> im_(arr2.view());
		image_view<rgb_color> im2(im);
		REQUIRE(same(im2.array_view(), im.array_view()));
		
		// construct const image_view(image_view)
		image_view<const rgb_color> im3(im);
		REQUIRE(same(im3.array_view(), im.array_view()));
		
		// construct const image_view(const image_view)
		image_view<const rgb_color> im4(im3);
		
		// move-construct image_view(image_view)
		image_view<rgb_color> im5(std::move(im));
		REQUIRE(same(im5.array_view(), im.array_view()));
		
		// image_view.reset(image_view)
		im5.reset(im2);
		REQUIRE(same(im5.array_view(), im2.array_view()));

		// const image_view.reset(image_view)
		im3.reset(im_);
		REQUIRE(same(im3.array_view(), im_.array_view()));

		// const image_view.reset(const image_view)
		im3.reset(im4);
		REQUIRE(same(im3.array_view(), im4.array_view()));
	}

	SECTION("from ndarray_view") {
		// construct image_view(ndarray_view)
		ndarray_view<2, rgb_color> ndvw = arr.view();
		image_view<rgb_color> im(ndvw);
		REQUIRE_FALSE(im.is_null());
		REQUIRE(same(im.array_view(), ndvw));
		REQUIRE(im.shape() == ndvw.shape());
		// ndvw and im must reference same memory
		REQUIRE(im.cv_mat()(3, 4) == ndvw[3][4]);
		ndvw[3][4] = c1;
		REQUIRE(im.cv_mat()(3, 4) == c1);
		im.cv_mat()(5, 8) = c2;
		REQUIRE(ndvw[5][8] == c2);

		// construct image_view(invalid ndarray_view)
		ndarray_view<2, rgb_color> ndvw3 = ndvw.section(ndvw.full_span(), make_ndptrdiff(2, 2));
		REQUIRE_FALSE(ndvw3.has_default_strides_without_padding());
		REQUIRE_THROWS(new image_view<rgb_color>(ndvw3));
	}
	
	SECTION("from const ndarray_view") {
		// construct const image_view(const ndarray_view)
		ndarray_view<2, const rgb_color> ndvw = arr.view();
		image_view<const rgb_color> im(ndvw);
		REQUIRE(same(im.array_view(), ndvw));
		
		// construct const image_view(ndarray_view)
		ndarray_view<2, rgb_color> ndvw2 = arr.view();
		image_view<const rgb_color> im2(ndvw2);
		REQUIRE(same(im2.array_view(), ndvw2));
	}
	
	SECTION("from opencv mat") {
		image_view<rgb_color> im(arr.view());
		auto cvmat = im.cv_mat();

		// construct image_view(cv_mat)
		image_view<rgb_color> im2(cvmat);
		// cvmat and im must reference same memory
		REQUIRE(im.cv_mat()(3, 4) == cvmat(3, 4));
		cvmat(3, 4) = c1;
		REQUIRE(im.cv_mat()(3, 4) == c1);
		im.cv_mat()(5, 8) = c2;
		REQUIRE(cvmat(5, 8) == c2);
		
		// construct const image_view(cv_mat)
		image_view<const rgb_color> im3(cvmat);
	}
}
