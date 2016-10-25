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

#ifndef MF_UTILITY_MEMORY_H_
#define MF_UTILITY_MEMORY_H_

#include "../common.h"
#include <cassert>
#include <cstddef>

namespace mf {


enum class memory_usage_advice {
	normal,
	sequential,
	random
};

/// Provide hint to operating system on how memory at `ptr` will be accessed.
void set_memory_usage_advice(void* ptr, std::size_t, memory_usage_advice);


/// Raw allocator, allocates given number of bytes.
class raw_allocator {
public:
	static std::size_t size_granularity() { return 1; }
	void* raw_allocate(std::size_t size, std::size_t align = 1);	
	void raw_deallocate(void* ptr, std::size_t size);
};


/// Ring allocator, allocates ring buffer memory.
/** Allocates given segment of memory, and maps additional same sized segment immediatley after it in virtual memory,
 ** which is mapped to the same allocated segment. In the allocated segment `seg`, `seg[i]` and `seg[i+n]` always
 ** map to the same data. The segment length `n` must be a multiple of the system page size. */
class raw_ring_allocator {
public:
	static std::size_t size_granularity();
	void* raw_allocate(std::size_t size, std::size_t align = 1);
	void raw_deallocate(void* ptr, std::size_t size);
};


/// Null allocator, allocates virtual memory which is not used.
/** Allocates segment of virtual memory which does not store data. Allocated memory may be read and written, but does
 ** not need to retain values. Used to allocate buffers that will not be used. OS-specific implementation may map
 ** virtual memory pages without allocating physical memory pages, when possible. */
class raw_null_allocator {
public:
	static std::size_t size_granularity() { return 1; }
	void* raw_allocate(std::size_t size, std::size_t align = 1);
	void raw_deallocate(void* ptr, std::size_t size);
};



}

#endif
