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
#include <vector>
#include <algorithm>
#include <mf/ndarray/ndarray_view.h>
#include <mf/ndarray/ndarray_format.h>
#include "../support/ndarray.h"

using namespace mf;
using namespace mf::test;


TEST_CASE("ndarray_view", "[nd][ndarray_view]") {
	constexpr std::ptrdiff_t l = sizeof(int);
	constexpr std::ptrdiff_t pad = l;
	constexpr std::size_t len = 3 * 4 * 4;
	std::vector<int> raw(len);
	for(int i = 0; i < len; ++i) raw[i] = i;


	SECTION("basics") {
		// default strides (row major)
		REQUIRE( (ndarray_view<1, int>::default_strides(make_ndsize(10))) == (ndptrdiff<1>{l}) );
		REQUIRE( (ndarray_view<2, int>::default_strides(make_ndsize(10, 10))) == (ndptrdiff<2>{10*l, l}) );
		REQUIRE( (ndarray_view<3, int>::default_strides(make_ndsize(4, 3, 2))) == (ndptrdiff<3>{3*2*l, 2*l, l}) );
		REQUIRE( (ndarray_view<3, int>::default_strides(make_ndsize(4, 3, 2), pad)) == (ndptrdiff<3>{3*2*(l+pad), 2*(l+pad), l+pad}) );
				
		// default strides view
		ndsize<3> shp{4, 3, 4};
		ndarray_view<3, int> a1(raw.data(), shp);
		REQUIRE(a1.start() == raw.data());
		REQUIRE(a1.shape() == shp);
		REQUIRE(a1.has_default_strides());
		REQUIRE(a1.default_strides_padding() == 0);
		REQUIRE(a1.has_default_strides_without_padding());

		// padded default strides view
		ndarray_view<3, int> a1pad(raw.data(), shp, ndarray_view<3, int>::default_strides(shp, pad));
		REQUIRE(a1pad.start() == raw.data());
		REQUIRE(a1pad.shape() == shp);
		REQUIRE(a1pad.has_default_strides());
		REQUIRE(a1pad.default_strides_padding() == pad);
		REQUIRE_FALSE(a1pad.has_default_strides_without_padding());
		
		// non-default strides
		ndptrdiff<3> str{4,2,1};
		ndarray_view<3, int> a2(raw.data(), shp, str);
		REQUIRE(a2.strides() == str);
		REQUIRE(a2.size() == 4*3*4);
		REQUIRE_FALSE(a2.has_default_strides());
		REQUIRE_THROWS(a2.default_strides_padding());
		REQUIRE_FALSE(a2.has_default_strides_without_padding());
		
		// comparison and assignment (shallow)
		ndarray_view<3, int> a3(raw.data() + 13, shp, str);
		REQUIRE(same(a1, a1));
		REQUIRE_FALSE(same(a1, a3));
		REQUIRE_FALSE(same(a3, a1));
		ndarray_view<3, int> a3_;
		a3_.reset(raw.data() + 13, shp, str);
		REQUIRE(same(a3_, a3));
		a3.reset(a1);
		REQUIRE(a3.start() == raw.data());
		REQUIRE(a3.shape() == shp);
		REQUIRE(a3.strides() == a3.default_strides(shp));
		REQUIRE(same(a3, a1));
		REQUIRE(same(a1, a3));
		
		// copy construction
		ndarray_view<3, int> a1copy = a1;
		REQUIRE(same(a1copy, a1));
		
		// const and non-const
		ndarray_view<3, const int> a1c = a1;
		REQUIRE(same(a1c, a1));
		REQUIRE(same(a1, a1c));
		a1c.reset(a1);
	}
	
	
	SECTION("null view; zero length view") {
		int placeholder[2] = {123, 123};
		ndarray_view<3, int> null_vw;
		ndarray_view<3, int> zero_size_vw(&placeholder[0], make_ndsize(0, 0, 0));

		SECTION("null view construction, assignment") {
			ndsize<3> shp{4, 3, 4};
			ndarray_view<3, int> a1(raw.data(), shp);
		
			ndarray_view<3, int> null_vw;
			REQUIRE(same(null_vw, ndarray_view<3, int>::null()));
			REQUIRE(null_vw.is_null());
			REQUIRE(! null_vw);
		
			REQUIRE_FALSE(a1.is_null());
			REQUIRE(a1);
			REQUIRE_FALSE(same(a1, null_vw));
			a1.reset();
			REQUIRE(a1.is_null());
			REQUIRE(! a1);
			REQUIRE(same(a1, null_vw));
		
			a1.reset(raw.data(), shp);
			REQUIRE_FALSE(a1.is_null());
			a1.reset(null_vw);
		}
		
		SECTION("null view attributes") {
			REQUIRE(null_vw.is_null());
			REQUIRE(! null_vw);
			
			REQUIRE(null_vw.start() == nullptr);
			REQUIRE(null_vw.shape() == make_ndsize(0, 0, 0));
			REQUIRE(null_vw.size() == 0);
			REQUIRE(null_vw.full_span() == make_ndspan(make_ndptrdiff(0, 0, 0)));

			REQUIRE(null_vw.begin() == null_vw.end());
			REQUIRE(same(null_vw, null_vw));
			REQUIRE_FALSE(same(null_vw, zero_size_vw));
		}
		
		SECTION("zero size view attributes") {
			REQUIRE_FALSE(zero_size_vw.is_null());
			REQUIRE(zero_size_vw);
			
			REQUIRE(zero_size_vw.start() == &placeholder[0]);
			REQUIRE(zero_size_vw.shape() == make_ndsize(0, 0, 0));
			REQUIRE(zero_size_vw.size() == 0);
			REQUIRE(zero_size_vw.full_span() == make_ndspan(make_ndptrdiff(0, 0, 0)));

			REQUIRE(null_vw.begin() == null_vw.end());

			ndarray_view<3, int> zero_size_vw2(&placeholder[1], make_ndsize(0, 0, 0));
			REQUIRE(same(zero_size_vw, zero_size_vw));
			REQUIRE_FALSE(same(zero_size_vw, zero_size_vw2));
		}
	}

	SECTION("1dim") {
		ndarray_view<1, int> arr1(raw.data(), ndsize<1>(len));

		SECTION("section") {
			// interval [2, 10[, with steps 1,2,3
			// testing sequence and shape (== number of elements)
			REQUIRE(arr1(2, 10).shape().front() == 8);
			REQUIRE(compare_sequence_(arr1(2, 10), { 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 }));
			REQUIRE(compare_sequence_(arr1(2, 10, 1), { 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 }));
			REQUIRE(arr1(2, 10, 2).shape().front() == 4);
			REQUIRE(compare_sequence_(arr1(2, 10, 2), { 0x02, 0x04, 0x06, 0x08 }));
			REQUIRE(arr1(2, 10, 3).shape().front() == 3);
			REQUIRE(compare_sequence_(arr1(2, 10, 3), { 0x02, 0x05, 0x08 }));

			// interval [2, 10[, with negative steps -1,-2,-3
			REQUIRE(arr1(2, 10, -1).shape().front() == 8);
			REQUIRE(compare_sequence_(arr1(2, 10, -1), { 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02 }));
			REQUIRE(arr1(2, 10, -2).shape().front() == 4);
			REQUIRE(compare_sequence_(arr1(2, 10, -2), { 0x08, 0x06, 0x04, 0x02 }));
			REQUIRE(arr1(2, 10, -3).shape().front() == 3);
			REQUIRE(compare_sequence_(arr1(2, 10, -3), { 0x08, 0x05, 0x02 }));

			// interval [2, 3[ = one element
			REQUIRE(compare_sequence_(arr1(2, 3), { 0x02 }));
			REQUIRE(arr1(2, 3).shape().front() == 1);
			REQUIRE(compare_sequence_(arr1(2, 3, 5), { 0x02 }));
			REQUIRE(arr1(2, 3, 5).shape().front() == 1);

			// too long step = one element
			ndarray_view<1, int> sec = arr1(0, 4);
			REQUIRE(compare_sequence_(sec(0, 4, 100), { 0x00 }));

			// negative start and end
			REQUIRE(compare_sequence_(sec(0, 4, 1), { 0x00, 0x01, 0x02, 0x03 }));
			REQUIRE(compare_sequence_(sec(0, -1, 1), { 0x00, 0x01, 0x02 }));
			REQUIRE(compare_sequence_(sec(0, -2, 1), { 0x00, 0x01 }));
			REQUIRE(compare_sequence_(sec(-3, 4, 1), { 0x01, 0x02, 0x03 }));
			REQUIRE(compare_sequence_(sec(-3, -1, 1), { 0x01, 0x02 }));
			REQUIRE(compare_sequence_(sec(-3, -2, 1), { 0x01 }));
		
			// start/end out of bounds
			REQUIRE_THROWS(sec(10, 4, 1));
			REQUIRE_THROWS(sec(-10, 4, 1));
			REQUIRE_THROWS(sec(0, 10, 1));
			REQUIRE_THROWS(sec(0, -10, 1));
		
			// start/end/step invalid
			REQUIRE_THROWS(sec(2, 2, 1));
			REQUIRE_THROWS(sec(3, 2, 1));
			REQUIRE_THROWS(sec(0, 4, 0));
		
			// whole, and single value
			REQUIRE(compare_sequence_(arr1(), raw));
			REQUIRE(compare_sequence_(arr1(1), { 0x01 }));
		}
		
		SECTION("deep assign, compare") {
			// compare to self
			REQUIRE(arr1 == arr1);
			REQUIRE(arr1.compare(arr1));
			REQUIRE_FALSE(arr1 != arr1);
			
			// different data with same values
			std::vector<int> raw_(len);
			for(int i = 0; i < len; ++i) raw_[i] = i;
			REQUIRE(raw_ == raw);
			ndarray_view<1, int> arr1_(raw_.data(), ndsize<1>(len));
			REQUIRE_FALSE(same(arr1, arr1_));
			REQUIRE_FALSE(same(arr1_, arr1));
			
			// comparing values
			REQUIRE(arr1 == arr1_);
			REQUIRE(arr1_ == arr1);
			REQUIRE(arr1.compare(arr1_));
			REQUIRE(arr1_.compare(arr1));
			REQUIRE_FALSE(arr1 != arr1_);
			REQUIRE_FALSE(arr1_ != arr1);
			
			// altering data
			raw[5] = 123;
			REQUIRE_FALSE(arr1 == arr1_);
			REQUIRE_FALSE(arr1_ == arr1);
			REQUIRE_FALSE(arr1.compare(arr1_));
			REQUIRE_FALSE(arr1_.compare(arr1));
			REQUIRE(arr1 != arr1_);
			REQUIRE(arr1_ != arr1);
			
			// assign values
			arr1_.assign(arr1);
			REQUIRE(raw_ == raw);
			REQUIRE(arr1 == arr1_);
			
			// assign section
			arr1(1, 4).assign(arr1(11, 14));
			REQUIRE(arr1(1, 4) == arr1(11, 14));
			REQUIRE(compare_sequence_(arr1(0, 15),
			{ 0, 11, 12, 13, 4, 123, 6, 7, 8, 9, 10, 11, 12, 13, 14 })); 
		}
	}


	SECTION("3dim") {
		ndarray_view<3, int> arr3(raw.data(), make_ndsize(3, 4, 4));
		ndarray_view<1, int> row = arr3[0][0];	
		// arr3:
		//
		// 00 01 02 03
		// 04 05 06 07
		// 08 09 0A 0B
		// 0C 0D 0E 0F
		// 
		//     10 11 12 13
		//     14 15 16 17
		//     18 19 1A 1B
		//     1C 1D 1E 1F
		// 
		//         20 21 22 23
		//         24 25 26 27
		//         28 29 2A 2B
		//         2C 2D 2E 2F	

		SECTION("subscript") {
			// subscript using [][][] and using at()
			REQUIRE(arr3[0][0][0] == 0x00);
			REQUIRE(arr3.at({ 0, 0, 0 }) == 0x00);
			REQUIRE(arr3[1][1][1] == 0x15);
			REQUIRE(arr3.at({ 1, 1, 1 }) == 0x15);
			REQUIRE(arr3[1][2][3] == 0x1B);
			REQUIRE(arr3.at({ 1, 2, 3 }) == 0x1B);
		
			// negative coordinate = warparound
			REQUIRE(arr3[-1][-1][-1] == 0x2F);
			REQUIRE(arr3.at({ -1, -1, -1 }) == 0x2F);
			REQUIRE(arr3[-2][-2][-2] == 0x1A);
			REQUIRE(arr3.at({ -2, -2, -2 }) == 0x1A);
			REQUIRE(arr3[-2][-3][-4] == 0x14);	
			REQUIRE(arr3.at({ -2, -3, -4 }) == 0x14);
			REQUIRE(arr3[-1][1][0] == 0x24);		
			REQUIRE(arr3.at({ -1, 1, 0 }) == 0x24);
			
			// modification
			arr3[0][0][0] = 123;
			REQUIRE(arr3[0][0][0] == 123);
			arr3[-1][-1][-1] = 456;
			REQUIRE(arr3[-1][-1][-1] == 456);
		}
		
		SECTION("deep assign, compare") {
			REQUIRE(arr3[0] != arr3[1]);
			arr3[0].assign(arr3[1]);
			REQUIRE(arr3[0] == arr3[1]);
			REQUIRE(arr3[0] != arr3[2]);
			arr3[0][1].assign(arr3[2][2]);
			REQUIRE(arr3[0][1] == arr3[2][2]);
			REQUIRE(arr3[0] != arr3[1]);
			arr3[0][1].assign(arr3[1][1]);
			REQUIRE(arr3[0] == arr3[1]);
			arr3[0][1][2] = 123;
			REQUIRE(arr3[0][0] == arr3[1][0]);
			REQUIRE(arr3[0] != arr3[1]);
		}
		
		SECTION("iterator") {
			// iterate through whole array
			REQUIRE(compare_sequence_(arr3, raw));
		
			REQUIRE(compare_sequence_(arr3[1], 
			{ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F }));
		
			REQUIRE(compare_sequence_(arr3[2][1], { 0x24, 0x25, 0x26, 0x27 }));
		
			// ++, --, +=, -= on iterator, test index, coordinates and value
			auto it = arr3.begin();
			REQUIRE(it.index() == 0);
			REQUIRE(it.coordinates() == make_ndptrdiff(0, 0, 0));
			REQUIRE(*it == 0x00);
			REQUIRE((++it).coordinates() == make_ndptrdiff(0, 0, 1));
			REQUIRE(it.index() == 1);
			REQUIRE(*it == 0x01);
			REQUIRE((++it).coordinates() == make_ndptrdiff(0, 0, 2));
			REQUIRE(it.index() == 2);
			REQUIRE(*it == 0x02);
			REQUIRE((it++).coordinates() == make_ndptrdiff(0, 0, 2));
			REQUIRE(it.index() == 3);
			REQUIRE(*it == 0x03);
			REQUIRE(it.coordinates() == make_ndptrdiff(0, 0, 3));
			REQUIRE((++it).coordinates() == make_ndptrdiff(0, 1, 0));
			REQUIRE(it.index() == 4);
			REQUIRE(*it == 0x04);
			it += 2;
			REQUIRE(it.coordinates() == make_ndptrdiff(0, 1, 2));
			REQUIRE(it.index() == 6);
			REQUIRE(*it == 0x06);
			REQUIRE((--it).coordinates() == make_ndptrdiff(0, 1, 1));
			REQUIRE(it.index() == 5);
			REQUIRE(*it == 0x05);
			it -= 3;
			REQUIRE(it.coordinates() == make_ndptrdiff(0, 0, 2));
			REQUIRE(it.index() == 2);
			REQUIRE(*it == 0x02);
		
			// second iterator, it2 != it
			// test ==, !=, >, >=, <, <=
			auto it2 = arr3.begin();
			REQUIRE(it != it2);
			REQUIRE(it2 != it);
			REQUIRE_FALSE(it == it2);
			REQUIRE_FALSE(it2 == it);
			REQUIRE(it > it2);
			REQUIRE_FALSE(it2 > it);
			REQUIRE(it >= it2);
			REQUIRE_FALSE(it2 >= it);
			REQUIRE(it2 < it);
			REQUIRE_FALSE(it < it2);
			REQUIRE(it2 <= it);
			REQUIRE_FALSE(it <= it2);
			it2++;
			it--;
			// now it == it2
			REQUIRE_FALSE(it != it2);
			REQUIRE_FALSE(it2 != it);
			REQUIRE(it == it2);
			REQUIRE(it2 == it);
			REQUIRE_FALSE(it > it2);
			REQUIRE_FALSE(it2 > it);
			REQUIRE(it >= it2);
			REQUIRE(it2 >= it);
			REQUIRE_FALSE(it2 < it);
			REQUIRE_FALSE(it < it2);
			REQUIRE(it2 <= it);
			REQUIRE(it <= it2);
		
			// [], +, -
			it2 += 4;
			REQUIRE(it[4] == *it2);
			REQUIRE(it + 4 == it2);
			REQUIRE(4 + it == it2);
			REQUIRE(it2 - 4 == it);
			REQUIRE(it2 - it == 4);
			REQUIRE(it - it2 == -4);
		}
		
		SECTION("section") {
			// interval [1,3[, for one dimension
			REQUIRE(arr3.shape() == make_ndsize(3, 4, 4));
			REQUIRE(arr3(1, 3, 1)()().shape() == make_ndsize(2, 4, 4));
			REQUIRE(arr3(1, 3, 1)()().strides() == make_ndptrdiff(0x10*l, 0x04*l, l));
			REQUIRE(compare_sequence_(arr3(1, 3, 1)()(), {
				0x10, 0x11, 0x12, 0x13,
				0x14, 0x15, 0x16, 0x17,
				0x18, 0x19, 0x1A, 0x1B,
				0x1C, 0x1D, 0x1E, 0x1F,
			
				0x20, 0x21, 0x22, 0x23,
				0x24, 0x25, 0x26, 0x27,
				0x28, 0x29, 0x2A, 0x2B,
				0x2C, 0x2D, 0x2E, 0x2F
			}));
			REQUIRE(arr3(1, 3, 1)()()[1][2][3] == 0x2B);
			REQUIRE(same( arr3(1, 3, 1)()(), arr3(1, 3, 1)() ));
			REQUIRE(same( arr3(1, 3, 1)()(), arr3(1, 3, 1) ));
			REQUIRE(same(
				arr3(1, 3, 1)()(),
				arr3.section(make_ndptrdiff(1, 0, 0), make_ndptrdiff(3, 4, 4), make_ndptrdiff(1, 1, 1))
			));
			REQUIRE(same(
				arr3(1, 3, 1)()(),
				arr3.section(make_ndspan(make_ndptrdiff(1, 0, 0), make_ndptrdiff(3, 4, 4)), make_ndptrdiff(1, 1, 1))
			));
		
			REQUIRE(arr3()(1, 3, 1)().shape() == make_ndsize(3, 2, 4));
			REQUIRE(arr3()(1, 3, 1)().strides() == make_ndptrdiff(0x10*l, 0x04*l, l));
			REQUIRE(compare_sequence_(arr3()(1, 3, 1)(), {
				0x04, 0x05, 0x06, 0x07,
				0x08, 0x09, 0x0A, 0x0B,
			
				0x14, 0x15, 0x16, 0x17,
				0x18, 0x19, 0x1A, 0x1B,
			
				0x24, 0x25, 0x26, 0x27,
				0x28, 0x29, 0x2A, 0x2B
			}));
			REQUIRE(arr3()(1, 3, 1)()[1][0][2] == 0x16);
			REQUIRE(same( arr3()(1, 3, 1)(), arr3()(1, 3, 1) ));
			REQUIRE(same(
				arr3()(1, 3, 1)(),
				arr3.section(make_ndptrdiff(0, 1, 0), make_ndptrdiff(3, 3, 4), make_ndptrdiff(1, 1, 1))
			));

			REQUIRE(arr3()()(1, 3, 1).shape() == make_ndsize(3, 4, 2));
			REQUIRE(arr3()()(1, 3, 1).strides() == make_ndptrdiff(0x10*l, 0x04*l, l));
			REQUIRE(compare_sequence_(arr3()()(1, 3, 1), {
				0x01, 0x02,
				0x05, 0x06,
				0x09, 0x0A,
				0x0D, 0x0E,
			
				0x11, 0x12,
				0x15, 0x16,
				0x19, 0x1A,
				0x1D, 0x1E,
			
				0x21, 0x22,
				0x25, 0x26,
				0x29, 0x2A,
				0x2D, 0x2E
			}));
			REQUIRE(arr3()()(1, 3, 1)[1][3][0] == 0x1D);
			REQUIRE(same(
				arr3()()(1, 3, 1),
				arr3.section(make_ndptrdiff(0, 0, 1), make_ndptrdiff(3, 4, 3), make_ndptrdiff(1, 1, 1))
			));

			// interval [1,n[ with strides in one dimension
			REQUIRE(arr3.shape() == make_ndsize(3, 4, 4));
			REQUIRE(arr3(1, 3, 2)()().shape() == make_ndsize(1, 4, 4)); // shape[0] == 3, so only 1
			REQUIRE(compare_sequence_(arr3(1, 3, 2)()(), {
				0x10, 0x11, 0x12, 0x13,
				0x14, 0x15, 0x16, 0x17,
				0x18, 0x19, 0x1A, 0x1B,
				0x1C, 0x1D, 0x1E, 0x1F,
			}));
			REQUIRE(arr3(1, 3, 2)()()[0][1][2] == 0x16);
			REQUIRE(same(
				arr3(1, 3, 2)()(),
				arr3.section(make_ndptrdiff(1, 0, 0), make_ndptrdiff(3, 4, 4), make_ndptrdiff(2, 1, 1))
			));
		
			REQUIRE(arr3()(1, 4, 2)().shape() == make_ndsize(3, 2, 4));
			REQUIRE(arr3()(1, 4, 2)().strides() == make_ndptrdiff(0x10*l, 2*0x04*l, l));
			REQUIRE(compare_sequence_(arr3()(1, 4, 2)(), {
				0x04, 0x05, 0x06, 0x07,
				0x0C, 0x0D, 0x0E, 0x0F,
			
				0x14, 0x15, 0x16, 0x17,
				0x1C, 0x1D, 0x1E, 0x1F,
			
				0x24, 0x25, 0x26, 0x27,
				0x2C, 0x2D, 0x2E, 0x2F
			}));
			REQUIRE(arr3()(1, 4, 2)()[1][0][3] == 0x17);
			REQUIRE(same(
				arr3()(1, 4, 2)(),
				arr3.section(make_ndptrdiff(0, 1, 0), make_ndptrdiff(3, 4, 4), make_ndptrdiff(1, 2, 1))
			));

			REQUIRE(arr3()()(1, 4, 2).shape() == make_ndsize(3, 4, 2));
			REQUIRE(arr3()()(1, 4, 2).strides() == make_ndptrdiff(0x10*l, 0x04*l, 2*l));
			REQUIRE(compare_sequence_(arr3()()(1, 4, 2), {
				0x01, 0x03,
				0x05, 0x07,
				0x09, 0x0B,
				0x0D, 0x0F,
			
				0x11, 0x13,
				0x15, 0x17,
				0x19, 0x1B,
				0x1D, 0x1F,
			
				0x21, 0x23,
				0x25, 0x27,
				0x29, 0x2B,
				0x2D, 0x2F
			}));
			REQUIRE(arr3()()(1, 4, 2)[2][0][1] == 0x23);
			REQUIRE(same(
				arr3()()(1, 4, 2),
				arr3.section(make_ndptrdiff(0, 0, 1), make_ndptrdiff(3, 4, 4), make_ndptrdiff(1, 1, 2))
			));

			// interval [1,3[ with reversal in one dimension
			REQUIRE(arr3.shape() == make_ndsize(3, 4, 4));
			REQUIRE(arr3(1, 3, -1)()().shape() == make_ndsize(2, 4, 4));
			REQUIRE(arr3(1, 3, -1)()().strides() == make_ndptrdiff(-0x10*l, 0x04*l, l));
			REQUIRE(compare_sequence_(arr3(1, 3, -1)()(), {			
				0x20, 0x21, 0x22, 0x23,
				0x24, 0x25, 0x26, 0x27,
				0x28, 0x29, 0x2A, 0x2B,
				0x2C, 0x2D, 0x2E, 0x2F,
			
				0x10, 0x11, 0x12, 0x13,
				0x14, 0x15, 0x16, 0x17,
				0x18, 0x19, 0x1A, 0x1B,
				0x1C, 0x1D, 0x1E, 0x1F
			}));
			REQUIRE(arr3(1, 3, -1)()()[1][2][3] == 0x1B);
			REQUIRE(same(
				arr3(1, 3, -1)()(),
				arr3.section(make_ndptrdiff(1, 0, 0), make_ndptrdiff(3, 4, 4), make_ndptrdiff(-1, 1, 1))
			));
		
			REQUIRE(arr3()(1, 3, -1)().shape() == make_ndsize(3, 2, 4));
			REQUIRE(arr3()(1, 3, -1)().strides() == make_ndptrdiff(0x10*l, -0x04*l, l));
			REQUIRE(compare_sequence_(arr3()(1, 3, -1)(), {
				0x08, 0x09, 0x0A, 0x0B,
				0x04, 0x05, 0x06, 0x07,
			
				0x18, 0x19, 0x1A, 0x1B,
				0x14, 0x15, 0x16, 0x17,
			
				0x28, 0x29, 0x2A, 0x2B,
				0x24, 0x25, 0x26, 0x27
			}));
			REQUIRE(arr3()(1, 3, -1)()[2][1][3] == 0x27);
			REQUIRE(same(
				arr3()(1, 3, -1)(),
				arr3.section(make_ndptrdiff(0, 1, 0), make_ndptrdiff(3, 3, 4), make_ndptrdiff(1, -1, 1))
			));

			REQUIRE(arr3()()(1, 3, -1).shape() == make_ndsize(3, 4, 2));
			REQUIRE(arr3()()(1, 3, -1).strides() == make_ndptrdiff(0x10*l, 0x04*l, -l));
			REQUIRE(compare_sequence_(arr3()()(1, 3, -1), {
				0x02, 0x01,
				0x06, 0x05,
				0x0A, 0x09,
				0x0E, 0x0D,
			
				0x12, 0x11,
				0x16, 0x15,
				0x1A, 0x19,
				0x1E, 0x1D,

				0x22, 0x21,
				0x26, 0x25,
				0x2A, 0x29,
				0x2E, 0x2D
			}));
			REQUIRE(arr3()()(1, 3, -1)[2][1][0] == 0x26);
			REQUIRE(same(
				arr3()()(1, 3, -1),
				arr3.section(make_ndptrdiff(0, 0, 1), make_ndptrdiff(3, 4, 3), make_ndptrdiff(1, 1, -1))
			));


			// multiple dimensions...
			auto sec1 = arr3(1, 3, 1)()(2, 4, 1);
			REQUIRE(sec1.shape() == make_ndsize(2, 4, 2));
			REQUIRE(sec1.strides() == make_ndptrdiff(0x10*l, 0x04*l, l));
			REQUIRE(compare_sequence_(sec1, {
				0x12, 0x13,
				0x16, 0x17,
				0x1A, 0x1B,
				0x1E, 0x1F,
			
				0x22, 0x23,
				0x26, 0x27,
				0x2A, 0x2B,
				0x2E, 0x2F
			}));
			REQUIRE(sec1[1][3][0] == 0x2E);
			REQUIRE(same( sec1,
				arr3.section(make_ndptrdiff(1, 0, 2), make_ndptrdiff(3, 4, 4), make_ndptrdiff(1, 1, 1)) ));
		
			auto sec2 = arr3(1, 3, 1)(0, 3, 2)(2, 4, 1);
			REQUIRE(sec2.shape() == make_ndsize(2, 2, 2));
			REQUIRE(sec2.strides() == make_ndptrdiff(0x10*l, 2*0x04*l, l));
			REQUIRE(compare_sequence_(sec2, {
				0x12, 0x13,
				0x1A, 0x1B,
		
				0x22, 0x23,
				0x2A, 0x2B
			}));
			REQUIRE(sec2[1][0][1] == 0x23);
			REQUIRE(sec2[1][0][0] == 0x22);
			REQUIRE(same( sec2,
				arr3.section(make_ndptrdiff(1, 0, 2), make_ndptrdiff(3, 3, 4), make_ndptrdiff(1, 2, 1)) ));

			auto sec3 = arr3(1, 3, 1)(0, 3, -2)(2, 4, 1);
			REQUIRE(sec3.shape() == make_ndsize(2, 2, 2));
			REQUIRE(sec3.strides() == make_ndptrdiff(0x10*l, -2*0x04*l, l));
			REQUIRE(compare_sequence_(sec3, {
				0x1A, 0x1B,
				0x12, 0x13,
			
				0x2A, 0x2B,
				0x22, 0x23
			}));
			REQUIRE(sec3[1][0][1] == 0x2B);
			REQUIRE(sec3[1][0][0] == 0x2A);
			REQUIRE(same( sec3,
				arr3.section(make_ndptrdiff(1, 0, 2), make_ndptrdiff(3, 3, 4), make_ndptrdiff(1, -2, 1)) ));

			auto sec4 = arr3(1, 3, -1)(0, 3, -2)(2, 4, -1);
			REQUIRE(sec4.shape() == make_ndsize(2, 2, 2));
			REQUIRE(sec4.strides() == make_ndptrdiff(-0x10*l, -2*0x04*l, -l));
			REQUIRE(compare_sequence_(sec4, {			
				0x2B, 0x2A,
				0x23, 0x22,

				0x1B, 0x1A,
				0x13, 0x12
			}));
			REQUIRE(sec4[1][0][1] == 0x1A);
			REQUIRE(sec4[1][0][0] == 0x1B);
			REQUIRE(same( sec4,
				arr3.section(make_ndptrdiff(1, 0, 2), make_ndptrdiff(3, 3, 4), make_ndptrdiff(-1, -2, -1)) ));
		}
		
		SECTION("index") {
			auto sec = arr3(1, 3, 1)(0, 3, -2)(2, 4, 1);
			// 1A 1B
			// 12 13
			//
			// 2A 2B
			// 22 23
			
			std::vector<int> values { 0x1A, 0x1B, 0x12, 0x13, 0x2A, 0x2B, 0x22, 0x23 };
			std::vector<ndptrdiff<3>> coordss {
				{0,0,0}, {0,0,1}, {0,1,0}, {0,1,1}, {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1}
			};
			for(std::ptrdiff_t index = 0; index < values.size(); ++index) {
				auto&& value = values[index];
				auto&& coords = coordss[index];
				REQUIRE(sec.coordinates_to_index(coords) == index);
				REQUIRE(sec.index_to_coordinates(index) == coords);
				REQUIRE(*sec.coordinates_to_pointer(coords) == value);
				REQUIRE(sec.at(coords) == value);
			}
		}
		
		SECTION("slice") {
			REQUIRE(same( arr3.slice(0, 0), arr3[0] ));
			REQUIRE(same( arr3.slice(1, 0), arr3[1] ));
			REQUIRE(same( arr3.slice(2, 0), arr3[2] ));

			REQUIRE(arr3.slice(1, 0)[2][3] == 0x1B);

			
			REQUIRE(arr3.slice(1, 0).dimension == 2);
			REQUIRE(arr3.slice(1, 0).shape() == make_ndsize(4, 4));
			
			REQUIRE(compare_sequence_(arr3.slice(0, 0), {
				0x00, 0x01, 0x02, 0x03,
				0x04, 0x05, 0x06, 0x07,
				0x08, 0x09, 0x0A, 0x0B,
				0x0C, 0x0D, 0x0E, 0x0F
			}));
			REQUIRE(arr3.slice(0, 0)[1][2] == 0x06);
			REQUIRE(compare_sequence_(arr3.slice(2, 0), {
				0x20, 0x21, 0x22, 0x23,
				0x24, 0x25, 0x26, 0x27,
				0x28, 0x29, 0x2A, 0x2B,
				0x2C, 0x2D, 0x2E, 0x2F
			}));
			REQUIRE(arr3.slice(2, 0)[1][2] == 0x26);

			REQUIRE(compare_sequence_(arr3.slice(0, 1), {
				0x00, 0x01, 0x02, 0x03,
				0x10, 0x11, 0x12, 0x13,
				0x20, 0x21, 0x22, 0x23
			}));
			REQUIRE(arr3.slice(0, 1)[1][2] == 0x12);
			REQUIRE(compare_sequence_(arr3.slice(3, 1), {
				0x0C, 0x0D, 0x0E, 0x0F,
				0x1C, 0x1D, 0x1E, 0x1F,
				0x2C, 0x2D, 0x2E, 0x2F
			}));
			REQUIRE(arr3.slice(3, 1)[1][2] == 0x1E);

			REQUIRE(compare_sequence_(arr3.slice(0, 2), {
				0x00, 0x04, 0x08, 0x0C,
				0x10, 0x14, 0x18, 0x1C,
				0x20, 0x24, 0x28, 0x2C
			}));
			REQUIRE(arr3.slice(0, 2)[1][2] == 0x18);
			REQUIRE(compare_sequence_(arr3.slice(2, 2), {
				0x02, 0x06, 0x0A, 0x0E,
				0x12, 0x16, 0x1A, 0x1E,
				0x22, 0x26, 0x2A, 0x2E
			}));
			REQUIRE(arr3.slice(2, 2)[1][2] == 0x1A);
		}
	}


	SECTION("conversion") {
		// Comparison
		std::vector<float> raw_f(len), raw_f2(len);
		for(int i = 0; i < len; ++i) raw_f[i] = float(i);
		for(int i = 0; i < len; ++i) raw_f2[i] = float(i + 1);

		ndarray_view<3, int> arr3(raw.data(), make_ndsize(3, 4, 4));
		ndarray_view<3, float> arr3_f(raw_f.data(), make_ndsize(3, 4, 4));
		ndarray_view<3, const float> arr3_fc(raw_f.data(), make_ndsize(3, 4, 4));
		ndarray_view<3, float> arr3_f2(raw_f2.data(), make_ndsize(3, 4, 4));

		REQUIRE(arr3.compare(arr3_f));
		REQUIRE(arr3 == arr3_f);
		REQUIRE(arr3_f == arr3);
		REQUIRE_FALSE(arr3 != arr3_f);
		REQUIRE_FALSE(arr3_f != arr3);
		REQUIRE(arr3_f.compare(arr3));
		
		REQUIRE(arr3.compare(arr3_fc));
		REQUIRE(arr3 == arr3_fc);
		REQUIRE_FALSE(arr3 != arr3_fc);
		
		REQUIRE_FALSE(arr3.compare(arr3_f2));
		REQUIRE_FALSE(arr3 == arr3_f2);
		REQUIRE(arr3 != arr3_f2);
		
		// Assignment
		arr3 = arr3_f2;
		REQUIRE(arr3.compare(arr3_f2));
		arr3.assign(arr3_fc);
		REQUIRE_FALSE(arr3.compare(arr3_f2));
	}
	
	
	SECTION("format") {
		SECTION("default") {
			ndarray_format afrm = make_ndarray_format<int>(100);
			REQUIRE(afrm.is_defined());
			REQUIRE(afrm.frame_size() == 100 * sizeof(int));
			REQUIRE(afrm.frame_alignment_requirement() == alignof(int));
			REQUIRE(afrm.length() == 100);
			REQUIRE(afrm.stride() == sizeof(int));
			REQUIRE(afrm.elem_size() == sizeof(int));
			REQUIRE(afrm.elem_alignment() == alignof(int));
			REQUIRE(afrm.elem_padding() == 0);

			afrm = make_ndarray_format<int>(100, pad);
			REQUIRE(afrm.is_defined());
			REQUIRE(afrm.frame_size() == 100 * (sizeof(int) + pad));
			REQUIRE(afrm.frame_alignment_requirement() == alignof(int));
			REQUIRE(afrm.length() == 100);
			REQUIRE(afrm.stride() == sizeof(int) + pad);
			REQUIRE(afrm.elem_size() == sizeof(int));
			REQUIRE(afrm.elem_alignment() == alignof(int));
			REQUIRE(afrm.elem_padding() == pad);

			REQUIRE_THROWS(make_ndarray_format<int>(100, sizeof(int)/2));
		}
		
		SECTION("from view") {
			auto shp = make_ndsize(3, 4, 4);
			auto str = ndarray_view<3, int>::default_strides(shp, pad);
			ndarray_view<3, int> vw(raw.data(), shp, str);
			
			SECTION("full") {
				ndarray_format afrm = format(vw);
				REQUIRE(afrm.is_defined());
				REQUIRE(afrm.frame_size() == (3*4*4) * str.back());
				REQUIRE(afrm.frame_alignment_requirement() == alignof(int));
				REQUIRE(afrm.length() == 3*4*4);
				REQUIRE(afrm.stride() == str.back());
				REQUIRE(afrm.elem_size() == sizeof(int));
				REQUIRE(afrm.elem_alignment() == alignof(int));
				REQUIRE(afrm.elem_padding() == pad);
			}
	
			SECTION("tail 2") {
				ndarray_format afrm = tail_format<2>(vw);
				REQUIRE(afrm.is_defined());
				REQUIRE(afrm.frame_size() == (4*4) * str.back());
				REQUIRE(afrm.frame_alignment_requirement() == alignof(int));
				REQUIRE(afrm.length() == 4*4);
				REQUIRE(afrm.stride() == str.back());
				REQUIRE(afrm.elem_size() == sizeof(int));
				REQUIRE(afrm.elem_alignment() == alignof(int));
				REQUIRE(afrm.elem_padding() == pad);
			}
			
			SECTION("tail 3") {
				ndarray_format afrm = tail_format<1>(vw);
				REQUIRE(afrm.is_defined());
				REQUIRE(afrm.frame_size() == 4 * str.back());
				REQUIRE(afrm.frame_alignment_requirement() == alignof(int));
				REQUIRE(afrm.length() == 4);
				REQUIRE(afrm.stride() == str.back());
				REQUIRE(afrm.elem_size() == sizeof(int));
				REQUIRE(afrm.elem_alignment() == alignof(int));
				REQUIRE(afrm.elem_padding() == pad);
			}
		}

		SECTION("undefined") {
			ndarray_format undef;
			REQUIRE(! undef.is_defined());
		}
		
		SECTION("assignment, comparison") {
			ndarray_format afrm = format(vw);
			ndarray_format afrm2 = afrm;
			REQUIRE(afrm == afrm2);
			REQUIRE_FALSE(afrm != afrm2);
			afrm = tail_format<2>(vw);
			REQUIRE_FALSE(afrm == afrm2);
			REQUIRE(afrm != afrm2);
		}
	}
}

