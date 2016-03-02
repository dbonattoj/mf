#include <catch.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "../src/ndarray_view.h"

using namespace mf;

template<typename Buffer>
static bool compare_sequence_forwards_(const Buffer& buf, const std::vector<int>& seq) {
	std::ostringstream str;

	auto vec_it = seq.begin();
	auto it = buf.begin();
	for(; (it != buf.end()) && (vec_it != seq.end()); ++it, ++vec_it) {
		str << std::hex << "got:" << *it << "  wanted:" << *vec_it << std::endl;
		if(*vec_it != *it) break;
	}
	if( (it == buf.end()) && (vec_it == seq.end()) ) return true;
	else if( (it == buf.end()) && (vec_it != seq.end()) ) str << "ended early" << std::endl;
	else if( (it != buf.end()) && (vec_it == seq.end()) ) str << "ended late" << std::endl;
	
	std::cout << "sequence mismatch: " << std::endl << str.str() << std::endl;	
	
	return false;
}


template<typename Buffer>
static bool compare_sequence_(const Buffer& buf, const std::vector<int>& seq) {
	if(! compare_sequence_forwards_(buf, seq)) return false;
	
	std::ostringstream str;

	auto vec_it = seq.end();
	auto it = buf.end();
	
	do {
		--it; --vec_it;
		str << std::hex << "got:" << *it << "  wanted:" << *vec_it << std::endl;
		if(*vec_it != *it) break;
	} while( (it != buf.begin()) && (vec_it != seq.begin()) );
	if( (it == buf.begin()) && (vec_it == seq.begin()) ) return true;
	else if( (it == buf.begin()) && (vec_it != seq.begin()) ) str << "arrived back early" << std::endl;
	else if( (it != buf.begin()) && (vec_it == seq.begin()) ) str << "arrived back late" << std::endl;
	
	std::cout << "sequence mismatch (backwards): " << std::endl << str.str() << std::endl;	
	
	return false;
}



TEST_CASE("ndarray_view", "[ndarray_view]") {
	constexpr std::ptrdiff_t l = sizeof(int);
	constexpr std::size_t len = 3 * 4 * 4;
	std::vector<int> raw(len);
	for(int i = 0; i < len; ++i) raw[i] = i;


	SECTION("basics") {
		// row major strides
		REQUIRE( (ndarray_view<1, int>::default_strides(ndsize<1>(10))) == (ndptrdiff<1>{l}) );
		REQUIRE( (ndarray_view<2, int>::default_strides(ndsize<2>(10, 10))) == (ndptrdiff<2>{10*l, l}) );
		REQUIRE( (ndarray_view<3, int>::default_strides(ndsize<3>(4, 3, 2))) == (ndptrdiff<3>{3*2*l, 2*l, l}) );
				
		// default strides = row major
		ndsize<3> shp(4, 3, 4);
		ndarray_view<3, int> a1(raw.data(), shp);
		REQUIRE(a1.start() == raw.data());
		REQUIRE(a1.shape() == shp);
		REQUIRE(a1.strides() == a1.default_strides(shp));
		
		// non-default strides
		ndptrdiff<3> str(4,2,1);
		ndarray_view<3, int> a2(raw.data(), shp, str);
		REQUIRE(a2.strides() == str);
		REQUIRE(a2.size() == 4*3*4);
		
		// comparison and assignment (shallow)
		ndarray_view<3, int> a3(raw.data() + 13, shp, str);
		REQUIRE(same(a1, a1));
		REQUIRE_FALSE(same(a1, a3));
		REQUIRE_FALSE(same(a3, a1));
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

	SECTION("3dim") {
		ndarray_view<3, int> arr3(raw.data(), ndsize<3>(3, 4, 4));
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
		
		SECTION("iterator") {
			// iterate through whole array
			REQUIRE(compare_sequence_(arr3, raw));
		
			REQUIRE(compare_sequence_(arr3[1], 
			{ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F }));
		
			REQUIRE(compare_sequence_(arr3[2][1], { 0x24, 0x25, 0x26, 0x27 }));
		
			// ++, --, +=, -= on iterator, test index, coordinates and value
			auto it = arr3.begin();
			REQUIRE(it.index() == 0);
			REQUIRE(it.coordinates() == ndptrdiff<3>(0, 0, 0));
			REQUIRE(*it == 0x00);
			REQUIRE((++it).coordinates() == ndptrdiff<3>(0, 0, 1));
			REQUIRE(it.index() == 1);
			REQUIRE(*it == 0x01);
			REQUIRE((++it).coordinates() == ndptrdiff<3>(0, 0, 2));
			REQUIRE(it.index() == 2);
			REQUIRE(*it == 0x02);
			REQUIRE((it++).coordinates() == ndptrdiff<3>(0, 0, 2));
			REQUIRE(it.index() == 3);
			REQUIRE(*it == 0x03);
			REQUIRE(it.coordinates() == ndptrdiff<3>(0, 0, 3));
			REQUIRE((++it).coordinates() == ndptrdiff<3>(0, 1, 0));
			REQUIRE(it.index() == 4);
			REQUIRE(*it == 0x04);
			it += 2;
			REQUIRE(it.coordinates() == ndptrdiff<3>(0, 1, 2));
			REQUIRE(it.index() == 6);
			REQUIRE(*it == 0x06);
			REQUIRE((--it).coordinates() == ndptrdiff<3>(0, 1, 1));
			REQUIRE(it.index() == 5);
			REQUIRE(*it == 0x05);
			it -= 3;
			REQUIRE(it.coordinates() == ndptrdiff<3>(0, 0, 2));
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
			REQUIRE(arr3.shape() == ndsize<3>(3, 4, 4));
			REQUIRE(arr3(1, 3, 1)()().shape() == ndsize<3>(2, 4, 4));
			REQUIRE(arr3(1, 3, 1)()().strides() == ndptrdiff<3>(0x10*l, 0x04*l, l));
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
			REQUIRE(same( arr3(1, 3, 1)()(), arr3.section(ndptrdiff<3>(1, 0, 0), ndptrdiff<3>(3, 4, 4), ndptrdiff<3>(1, 1, 1)) ));
		
			REQUIRE(arr3()(1, 3, 1)().shape() == ndsize<3>(3, 2, 4));
			REQUIRE(arr3()(1, 3, 1)().strides() == ndptrdiff<3>(0x10*l, 0x04*l, l));
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
			REQUIRE(same( arr3()(1, 3, 1)(), arr3.section(ndptrdiff<3>(0, 1, 0), ndptrdiff<3>(3, 3, 4), ndptrdiff<3>(1, 1, 1)) ));

			REQUIRE(arr3()()(1, 3, 1).shape() == ndsize<3>(3, 4, 2));
			REQUIRE(arr3()()(1, 3, 1).strides() == ndptrdiff<3>(0x10*l, 0x04*l, l));
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
			REQUIRE(same( arr3()()(1, 3, 1), arr3.section(ndptrdiff<3>(0, 0, 1), ndptrdiff<3>(3, 4, 3), ndptrdiff<3>(1, 1, 1)) ));

			// interval [1,n[ with strides in one dimension
			REQUIRE(arr3.shape() == ndsize<3>(3, 4, 4));
			REQUIRE(arr3(1, 3, 2)()().shape() == ndsize<3>(1, 4, 4)); // shape[0] == 3, so only 1
			REQUIRE(compare_sequence_(arr3(1, 3, 2)()(), {
				0x10, 0x11, 0x12, 0x13,
				0x14, 0x15, 0x16, 0x17,
				0x18, 0x19, 0x1A, 0x1B,
				0x1C, 0x1D, 0x1E, 0x1F,
			}));
			REQUIRE(arr3(1, 3, 2)()()[0][1][2] == 0x16);
			REQUIRE(same( arr3(1, 3, 2)()(), arr3.section(ndptrdiff<3>(1, 0, 0), ndptrdiff<3>(3, 4, 4), ndptrdiff<3>(2, 1, 1)) ));
		
			REQUIRE(arr3()(1, 4, 2)().shape() == ndsize<3>(3, 2, 4));
			REQUIRE(arr3()(1, 4, 2)().strides() == ndptrdiff<3>(0x10*l, 2*0x04*l, l));
			REQUIRE(compare_sequence_(arr3()(1, 4, 2)(), {
				0x04, 0x05, 0x06, 0x07,
				0x0C, 0x0D, 0x0E, 0x0F,
			
				0x14, 0x15, 0x16, 0x17,
				0x1C, 0x1D, 0x1E, 0x1F,
			
				0x24, 0x25, 0x26, 0x27,
				0x2C, 0x2D, 0x2E, 0x2F
			}));
			REQUIRE(arr3()(1, 4, 2)()[1][0][3] == 0x17);
			REQUIRE(same( arr3()(1, 4, 2)(), arr3.section(ndptrdiff<3>(0, 1, 0), ndptrdiff<3>(3, 4, 4), ndptrdiff<3>(1, 2, 1)) ));

			REQUIRE(arr3()()(1, 4, 2).shape() == ndsize<3>(3, 4, 2));
			REQUIRE(arr3()()(1, 4, 2).strides() == ndptrdiff<3>(0x10*l, 0x04*l, 2*l));
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
			REQUIRE(same( arr3()()(1, 4, 2), arr3.section(ndptrdiff<3>(0, 0, 1), ndptrdiff<3>(3, 4, 4), ndptrdiff<3>(1, 1, 2)) ));

			// interval [1,3[ with reversal in one dimension
			REQUIRE(arr3.shape() == ndsize<3>(3, 4, 4));
			REQUIRE(arr3(1, 3, -1)()().shape() == ndsize<3>(2, 4, 4));
			REQUIRE(arr3(1, 3, -1)()().strides() == ndptrdiff<3>(-0x10*l, 0x04*l, l));
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
			REQUIRE(same( arr3(1, 3, -1)()(), arr3.section(ndptrdiff<3>(1, 0, 0), ndptrdiff<3>(3, 4, 4), ndptrdiff<3>(-1, 1, 1)) ));
		
			REQUIRE(arr3()(1, 3, -1)().shape() == ndsize<3>(3, 2, 4));
			REQUIRE(arr3()(1, 3, -1)().strides() == ndptrdiff<3>(0x10*l, -0x04*l, l));
			REQUIRE(compare_sequence_(arr3()(1, 3, -1)(), {
				0x08, 0x09, 0x0A, 0x0B,
				0x04, 0x05, 0x06, 0x07,
			
				0x18, 0x19, 0x1A, 0x1B,
				0x14, 0x15, 0x16, 0x17,
			
				0x28, 0x29, 0x2A, 0x2B,
				0x24, 0x25, 0x26, 0x27
			}));
			REQUIRE(arr3()(1, 3, -1)()[2][1][3] == 0x27);
			REQUIRE(same( arr3()(1, 3, -1)(), arr3.section(ndptrdiff<3>(0, 1, 0), ndptrdiff<3>(3, 3, 4), ndptrdiff<3>(1, -1, 1)) ));

			REQUIRE(arr3()()(1, 3, -1).shape() == ndsize<3>(3, 4, 2));
			REQUIRE(arr3()()(1, 3, -1).strides() == ndptrdiff<3>(0x10*l, 0x04*l, -l));
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
			REQUIRE(same( arr3()()(1, 3, -1), arr3.section(ndptrdiff<3>(0, 0, 1), ndptrdiff<3>(3, 4, 3), ndptrdiff<3>(1, 1, -1)) ));


			// multiple dimensions...
			auto sec1 = arr3(1, 3, 1)()(2, 4, 1);
			REQUIRE(sec1.shape() == ndsize<3>(2, 4, 2));
			REQUIRE(sec1.strides() == ndptrdiff<3>(0x10*l, 0x04*l, l));
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
			REQUIRE(same( sec1, arr3.section(ndptrdiff<3>(1, 0, 2), ndptrdiff<3>(3, 4, 4), ndptrdiff<3>(1, 1, 1)) ));
		
			auto sec2 = arr3(1, 3, 1)(0, 3, 2)(2, 4, 1);
			REQUIRE(sec2.shape() == ndsize<3>(2, 2, 2));
			REQUIRE(sec2.strides() == ndptrdiff<3>(0x10*l, 2*0x04*l, l));
			REQUIRE(compare_sequence_(sec2, {
				0x12, 0x13,
				0x1A, 0x1B,
		
				0x22, 0x23,
				0x2A, 0x2B
			}));
			REQUIRE(sec2[1][0][1] == 0x23);
			REQUIRE(sec2[1][0][0] == 0x22);
			REQUIRE(same( sec2, arr3.section(ndptrdiff<3>(1, 0, 2), ndptrdiff<3>(3, 3, 4), ndptrdiff<3>(1, 2, 1)) ));

			auto sec3 = arr3(1, 3, 1)(0, 3, -2)(2, 4, 1);
			REQUIRE(sec3.shape() == ndsize<3>(2, 2, 2));
			REQUIRE(sec3.strides() == ndptrdiff<3>(0x10*l, -2*0x04*l, l));
			REQUIRE(compare_sequence_(sec3, {
				0x1A, 0x1B,
				0x12, 0x13,
			
				0x2A, 0x2B,
				0x22, 0x23
			}));
			REQUIRE(sec3[1][0][1] == 0x2B);
			REQUIRE(sec3[1][0][0] == 0x2A);
			REQUIRE(same( sec3, arr3.section(ndptrdiff<3>(1, 0, 2), ndptrdiff<3>(3, 3, 4), ndptrdiff<3>(1, -2, 1)) ));

			auto sec4 = arr3(1, 3, -1)(0, 3, -2)(2, 4, -1);
			REQUIRE(sec4.shape() == ndsize<3>(2, 2, 2));
			REQUIRE(sec4.strides() == ndptrdiff<3>(-0x10*l, -2*0x04*l, -l));
			REQUIRE(compare_sequence_(sec4, {			
				0x2B, 0x2A,
				0x23, 0x22,

				0x1B, 0x1A,
				0x13, 0x12
			}));
			REQUIRE(sec4[1][0][1] == 0x1A);
			REQUIRE(sec4[1][0][0] == 0x1B);
			REQUIRE(same( sec4, arr3.section(ndptrdiff<3>(1, 0, 2), ndptrdiff<3>(3, 3, 4), ndptrdiff<3>(-1, -2, -1)) ));
		}
		
		SECTION("index") {
			auto sec = arr3(1, 3, 1)(0, 3, -2)(2, 4, 1);
			// 1A 1B
			// 12 13
			//
			// 2A 2B
			// 22 23
			
			std::vector<int> values { 0x1A, 0x1B, 0x12, 0x13, 0x2A, 0x2B, 0x22, 0x23 };
			std::vector<ndptrdiff<3>> coordss { {0,0,0}, {0,0,1}, {0,1,0}, {0,1,1}, {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1} };
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
			REQUIRE(arr3.slice(1, 0).shape() == ndsize<2>(4, 4));
			
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
	}
		
	
	SECTION("objects") {
		struct base {
			int b;
		};
		struct derived : base {
			int d;
		};
		
		ndsize<2> shp(10, 20);
		std::vector<derived> raw(shp.product());
		for(std::ptrdiff_t i = 0; i < raw.size(); ++i) {
			raw[i].b = i;
			raw[i].d = 2*i;
		}
		
		ndarray_view<2, derived> arr(raw.data(), shp);
		REQUIRE(arr.shape() == shp);
		REQUIRE(arr.strides() == ndptrdiff<2>( 20*sizeof(derived) ,sizeof(derived)) );
		
		SECTION("downcast") {
			ndarray_view<2, base> arr2 = ndarray_view_cast<base>(arr);
			auto raw_it = raw.begin();
			auto it = arr2.begin();
			for(; (it != arr2.end()) && (raw_it != raw.end()); it++, raw_it++) {
				REQUIRE(raw_it->b == it->b);
				it->b *= 3;
			}
			
			for(std::ptrdiff_t i = 0; i < raw.size(); ++i) {
				REQUIRE(raw[i].b == 3*i);
				REQUIRE(raw[i].d == 2*i);
			}
		}
	}
}