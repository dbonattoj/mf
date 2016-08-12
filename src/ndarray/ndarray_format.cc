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

#include "ndarray_format.h"
#include "../utility/misc.h"
#include <cstring>
#include <cstdint>

namespace mf {

namespace {

// TODO benchmark optimization

template<typename Word>
class strided_memory_optimization_ {
private:
	static constexpr std::size_t word_size = sizeof(Word);

	static bool compare_(const Word* a_begin, const Word* b_begin, std::ptrdiff_t stride, std::size_t size) {
		auto a_end = a_begin + size;
		auto a = a_begin;
		auto b = b_begin;
		for(; a != a_end; a += stride, b += stride)
			if(*a != *b) return false;
		return true;
	}
	
	inline static void assign_(Word* a_begin, const Word* b_begin, std::ptrdiff_t stride, std::size_t size) {
		auto a_end = a_begin + size;
		auto a = a_begin;
		auto b = b_begin;
		for(; a != a_end; a += stride, b += stride)
			*a = *b;
	}

public:
	inline static bool compare(const void* a, const void* b, std::ptrdiff_t stride, std::size_t size) {
		Assert_crit(is_multiple_of(stride, word_size));
		Assert_crit(is_multiple_of(size, word_size));
		return compare_(
			static_cast<const Word*>(a),
			static_cast<const Word*>(b),
			stride / word_size,
			size / word_size
		);
	}
	
	inline static void assign(void* destination, const void* origin, std::ptrdiff_t stride, std::size_t size) {
		Assert_crit(is_multiple_of(stride, word_size));
		Assert_crit(is_multiple_of(size, word_size));
		assign_(
			static_cast<Word*>(destination),
			static_cast<const Word*>(origin),
			stride / word_size,
			size / word_size
		);
	}
};

}


bool operator==(const ndarray_format& a, const ndarray_format& b) {
	return
		(a.elem_size() == b.elem_size()) &&
		(a.elem_alignment() == b.elem_alignment()) &&
		(a.length() == b.length()) &&
		(a.stride() == b.stride());
}


bool operator!=(const ndarray_format& a, const ndarray_format& b) {
	return
		(a.elem_size() != b.elem_size()) ||
		(a.elem_alignment() != b.elem_alignment()) ||
		(a.length() != b.length()) ||
		(a.stride() != b.stride());
}


bool ndarray_data_compare(const void* a_raw, const void* b_raw, const ndarray_format& format) {
	std::size_t frame_len = format.frame_size();
	std::size_t elem_len = format.elem_size();
	std::ptrdiff_t stride = format.stride();
		
	if(format.is_contiguous()) {
		return (std::memcmp(a_raw, b_raw, frame_len) == 0);/*
	} else if(elem_len == 8 && is_multiple_of(stride, 8)) {
		return strided_memory_optimization_<std::uint64_t>::compare(a_raw, b_raw, stride, frame_len);
	} else if(elem_len == 4 && is_multiple_of(stride, 4)) {
		return strided_memory_optimization_<std::uint32_t>::compare(a_raw, b_raw, stride, frame_len);
	} else if(elem_len == 2 && is_multiple_of(stride, 2)) {
		return strided_memory_optimization_<std::uint16_t>::compare(a_raw, b_raw, stride, frame_len);
	} else if(elem_len == 1 && is_multiple_of(stride, 1)) {
		return strided_memory_optimization_<std::uint8_t>::compare(a_raw, b_raw, stride, frame_len);*/
	} else {
		auto a_raw_end = advance_raw_ptr(a_raw, frame_len);
		while(a_raw != a_raw_end) {
			if(std::memcmp(a_raw, b_raw, elem_len) != 0) return false;
			a_raw = advance_raw_ptr(a_raw, stride);
			b_raw = advance_raw_ptr(b_raw, stride);
		}
		return true;
	}
}


void ndarray_data_copy(void* dest_raw, const void* origin_raw, const ndarray_format& format) {
	std::size_t frame_len = format.frame_size();
	std::size_t elem_len = format.elem_size();
	std::ptrdiff_t stride = format.stride();

	if(format.is_contiguous()) {
		std::memcpy(dest_raw, origin_raw, frame_len);/*
	} else if(elem_len == 8 && is_multiple_of(stride, 8)) {
		strided_memory_optimization_<std::uint64_t>::assign(dest_raw, origin_raw, stride, frame_len);
	} else if(elem_len == 4 && is_multiple_of(stride, 4)) {
		strided_memory_optimization_<std::uint32_t>::assign(dest_raw, origin_raw, stride, frame_len);
	} else if(elem_len == 2 && is_multiple_of(stride, 2)) {
		strided_memory_optimization_<std::uint16_t>::assign(dest_raw, origin_raw, stride, frame_len);
	} else if(elem_len == 1 && is_multiple_of(stride, 1)) {
		strided_memory_optimization_<std::uint8_t>::assign(dest_raw, origin_raw, stride, frame_len);*/
	} else {
		auto origin_raw_end = advance_raw_ptr(origin_raw, frame_len);
		while(origin_raw != origin_raw_end) {
			std::memcpy(dest_raw, origin_raw, elem_len);
			origin_raw = advance_raw_ptr(origin_raw, stride);
			dest_raw = advance_raw_ptr(dest_raw, stride);
		}
	}
}



}
