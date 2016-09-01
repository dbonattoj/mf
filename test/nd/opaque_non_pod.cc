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
#include <mf/nd/opaque/ndarray_opaque.h>
#include <mf/nd/opaque_format/opaque_format.h>
#include <mf/nd/opaque_format/opaque_object_format.h>
#include <cstdint>

using namespace mf;

struct concrete_data {
	unsigned data:7;
	bool constructed:1;
};

static concrete_data& val(void* frame) { return *reinterpret_cast<concrete_data*>(frame); }
static const concrete_data& val(const void* frame) { return *reinterpret_cast<const concrete_data*>(frame); }

static int construction_counter = 0;

static void verify_constructed_(const ndarray_view_opaque<2, false>& vw) {
	const auto& shp = vw.shape();
	for(const auto& coord : make_ndspan(shp)) {
		concrete_data frame = val(vw.at(coord).start());
		REQUIRE(frame.constructed);
	}
}

static void verify_destructed_(const ndarray_view_opaque<2, false>& vw) {
	const auto& shp = vw.shape();
	for(const auto& coord : make_ndspan(shp)) {
		concrete_data frame = val(vw.at(coord).start());
		REQUIRE_FALSE(frame.constructed);
	}
}

class custom_object {
public:
	static int counter;
	
	custom_object() { counter++; }
	~custom_object() { counter--; }
	custom_object& operator=(const custom_object&) = default;
	bool operator==(const custom_object&) const { return true; }
};

int custom_object::counter = 0;

class custom_frame_format : public opaque_format {
public:
	custom_frame_format() {
		set_frame_size_(4);
		set_frame_alignment_requirement_(4);
		set_pod_(false);
		set_contiguous_(true);
	}
	
	bool compare(const opaque_format& frm) const override {
		return (typeid(frm) == typeid(custom_frame_format));
	}

	void copy_frame(frame_ptr destination, const_frame_ptr origin) const override {
		REQUIRE(val(destination).constructed);
		REQUIRE(val(origin).constructed);
		val(destination).data = val(origin).data;
	}
	
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const override {
		REQUIRE(val(a).constructed);
		REQUIRE(val(b).constructed);
		return (val(a).data == val(b).data);
	}
	
	void construct_frame(frame_ptr frame) const override {
		val(frame).constructed = true;
		construction_counter++;
	}
	
	void destruct_frame(frame_ptr frame) const override {
		REQUIRE(val(frame).constructed);
		val(frame).constructed = false;
		construction_counter--;
	}
};

TEST_CASE("non-pod frames", "[nd][ndarray_opaque]") {
	SECTION("custom_frame_format") {
		custom_frame_format frm, frm_;
		ndsize<2> shape{3, 4};
		ndsize<2> shape_{5, 5};
		
		REQUIRE(frm.is_contiguous());
		REQUIRE_FALSE(frm.is_pod());
		REQUIRE_FALSE(frm.is_contiguous_pod());
	
		REQUIRE(construction_counter == 0);
		{
			// construction with shape
			ndarray_opaque<2> arr(shape, frm);
			REQUIRE(arr.allocated_byte_size() >= arr.size()*frm.frame_size());
			verify_constructed_(arr.cview());
			ndarray_opaque<2> arr_(shape_, frm_);
			REQUIRE(construction_counter > 0);
		
			// construction from view
			ndarray_opaque<2> arr2(arr.cview());
			verify_constructed_(arr2.cview());
		
			// assignment from view
			arr = arr2.view();
			verify_constructed_(arr.cview());
		
			// assignment from array
			arr2 = arr_.cview();
			verify_constructed_(arr2.cview());
			
			// destruction of arr, arr2, arr_
		}
		REQUIRE(construction_counter == 0);
	}
	
	
	SECTION("custom_object frames") {
		opaque_object_format<custom_object> frm, frm_;
		ndsize<2> shape{3, 4};
		ndsize<2> shape_{5, 5};
		
		REQUIRE(frm.is_contiguous());
		REQUIRE_FALSE(frm.is_pod());
		REQUIRE_FALSE(frm.is_contiguous_pod());
	
		REQUIRE(custom_object::counter == 0);
		{
			ndarray_opaque<2> arr(shape, frm);
			REQUIRE(arr.allocated_byte_size() >= arr.size()*frm.frame_size());
			ndarray_opaque<2> arr_(shape_, frm_);
			REQUIRE(custom_object::counter > 0);
			ndarray_opaque<2> arr2(arr.cview());
			arr = arr2.view();
			arr2 = arr_.cview();
		}
		REQUIRE(custom_object::counter == 0);
	}
}
