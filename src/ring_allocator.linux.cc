#include "utility/os.h"
#ifdef MF_OS_LINUX

#include "ring_allocator.h"
#include "utility/memory.h"
#include "utility/misc.h"
#include <cerrno>
#include <system_error>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <cstdint>
#include <cassert>

namespace mf {

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
