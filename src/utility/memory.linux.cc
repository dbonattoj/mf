#include "os.h"
#ifdef MF_OS_LINUX

#include "memory.h"

#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdexcept>
#include <cassert>
#include <cstdint>
#include <cerrno>
#include <system_error>
#include <stdio.h>

#include "misc.h"


namespace mf {

std::size_t system_page_size() {
	return sysconf(_SC_PAGESIZE);
}

void set_memory_usage_advice(void* buf, std::size_t len, memory_usage_advice adv) {
	switch(adv) {
	case memory_usage_advice::normal:
		madvise(buf, len, MADV_NORMAL);
		break;
	case memory_usage_advice::sequential:
		madvise(buf, len, MADV_SEQUENTIAL);
		break;
	case memory_usage_advice::random:
		madvise(buf, len, MADV_RANDOM);
		break;
	return;	
	}
}


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
	


void* raw_ring_allocator::raw_allocate(std::size_t size, std::size_t align) {	
	std::size_t page_size = system_page_size();
	
	if(size % page_size != 0) throw std::invalid_argument("size must be multiple of page size");
	if(page_size % align != 0) throw std::invalid_argument("requested alignment must be divisor of page size"); 

	int status;
	void* ptr;
	
	FILE* file = ::tmpfile();
	if(file == nullptr)
		throw std::system_error(errno, std::system_category(), "ring allocator tmpfile failed");
	
	int fd = ::fileno(file);
	
	status = ::ftruncate(fd, size);
	if(status != 0)
		throw std::system_error(errno, std::system_category(), "ring allocator ftruncate failed");
		
	void* base = ::mmap(nullptr, size * 2, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0); // ...aligns to page size
	if(base == MAP_FAILED)
		throw std::system_error(errno, std::system_category(), "ring allocator mmap failed (whole region)");
		
	assert(reinterpret_cast<std::uintptr_t>(base) % align == 0);

	ptr = ::mmap(base, size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED)
		throw std::system_error(errno, std::system_category(), "ring allocator mmap failed (base)");

	ptr = ::mmap(advance_raw_ptr(base, size), size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED)
		throw std::system_error(errno, std::system_category(), "ring allocator mmap failed (mirror)");
	
	status = ::close(fd);
	if(status != 0)
		throw std::system_error(errno, std::system_category(), "ring allocator close failed");
	
	return base;
}


void raw_ring_allocator::raw_deallocate(void* base, std::size_t size) {
	::munmap(base, size * 2);
}



}

#endif
