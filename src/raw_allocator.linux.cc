#include "utility/os.h"
#ifdef MF_OS_LINUX

#include "raw_allocator.h"
#include <stdlib.h>
#include <stdexcept>
#include <cassert>
#include <cstdint>
#include "debug.h"

namespace mf {

void* raw_allocator::raw_allocate(std::size_t size, std::size_t align) {
	// for posix_memalign, alignment must be multiple of sizeof(void*) AND power of 2
	std::size_t actual_align = sizeof(void*);
	while(actual_align < align) actual_align *= 2;
	
	void* ptr = nullptr;
	int err = ::posix_memalign(&ptr, actual_align, size);
	if(err != 0) throw std::bad_alloc();
	assert(reinterpret_cast<std::uintptr_t>(ptr) % align == 0);
	return ptr;
}


void raw_allocator::raw_deallocate(void* ptr, std::size_t size) {
	::free(ptr);
}
	

}

#endif
