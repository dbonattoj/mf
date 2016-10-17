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

#include "os.h"
#ifdef MF_OS_WINDOWS

#include "memory.h"
#include <windows.h>
#include <malloc.h>
#include <cstdlib>
#include "../utility/misc.h"

#include <iostream>

namespace mf {
	
namespace {

const ::SYSTEM_INFO& system_info_() {
	static ::SYSTEM_INFO info;
	static bool loaded = false;
	if(! loaded) {
		::GetSystemInfo(&info);
		loaded = true;
	}
	return info;
}

}

std::size_t system_page_size() {
	//return system_info_().dwPageSize;
	return system_info_().dwAllocationGranularity;
}

void set_memory_usage_advice(void* buf, std::size_t len, memory_usage_advice adv) {
	// Not available on Windows
	return;
}



void* raw_allocator::raw_allocate(std::size_t size, std::size_t align) {
	// for _aligned_malloc, alignment must be power of 2
	std::size_t actual_align = 1;
	while(actual_align < align) actual_align *= 2;

	// for _aligned_malloc, alignment must be power of 2
	//void* ptr = ::_aligned_malloc(size, actual_align);
	void* ptr = aligned_alloc(actual_align, size);
	Assert(reinterpret_cast<std::uintptr_t>(ptr) % align == 0);

	return ptr;
}


void raw_allocator::raw_deallocate(void* ptr, std::size_t size) {
	//::_aligned_free(ptr);
	free(ptr);
}



void* raw_ring_allocator::raw_allocate(std::size_t size, std::size_t align) {	
	std::size_t alloc_granularity = system_info_().dwAllocationGranularity;
	
	std::cerr << "alloc gran: " << alloc_granularity << std::endl;
	
	if(size % alloc_granularity != 0)
		throw std::invalid_argument("size must be multiple of allocation granularity");
	if(alloc_granularity % align != 0)
		throw std::invalid_argument("requested alignment must be divisor of allocation granularity"); 

	HANDLE double_mapping = ::CreateFileMapping(
		INVALID_HANDLE_VALUE,
		nullptr,
		PAGE_READWRITE,
		0, 2 * size,
		NULL
	);
	if(double_mapping == nullptr) std::runtime_error("CreateFileMapping failed"); // TODO windows system error exception

	LPVOID base = ::VirtualAlloc(
		nullptr,
		2 * size,
		MEM_RESERVE,
		PAGE_READWRITE
	);
	if(base == nullptr) std::runtime_error("VirtualAlloc failed");

	/*
	LPVOID base = ::MapViewOfFile(
		double_mapping,
		FILE_MAP_WRITE,
		0, 0,
		2 * size
	);
	if(base == nullptr) std::runtime_error("MapViewOfFile (first) failed");

	UnmapViewOfFile(base);

	// assuming that the memory range [base, base + 2*size[ remains safe between these calls
	// TODO retry if not
	*/


	LPVOID actual_base = ::MapViewOfFileEx(
		double_mapping,
		FILE_MAP_WRITE,
		0, 0,
		size,
		base
	);
	if(actual_base == nullptr) std::runtime_error("MapViewOfFile (base) failed");
	Assert(actual_base == base);

	LPVOID actual_mirror = ::MapViewOfFileEx(
		double_mapping,
		FILE_MAP_WRITE,
		0, 0,
		size,
		advance_raw_ptr(base, size)
	);
	if(actual_mirror == nullptr) std::runtime_error("MapViewOfFile (mirror) failed");

	::CloseHandle(double_mapping);
	// if will close only when the views get unmapped, which is done in raw_deallocate()

	return static_cast<void*>(base);
}


void raw_ring_allocator::raw_deallocate(void* base, std::size_t size) {
	::UnmapViewOfFile(base);
	::UnmapViewOfFile(advance_raw_ptr(base, size));
	::VirtualFree(base, 2 * size, MEM_RELEASE);
}


void* raw_null_allocator::raw_allocate(std::size_t size, std::size_t align) {
	return ::malloc(size);
}


void raw_null_allocator::raw_deallocate(void* base, std::size_t size) {
	::free(base);
}


}

#endif
