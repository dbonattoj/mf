#include "util/os.h"
#ifdef MF_OS_DARWIN

#include "ring_allocator.h"
#include "util/memory.h"
#include "util.h"
#include <cerrno>
#include <system_error>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

namespace mf {

void* detail::ring_allocator_base::raw_allocate(std::size_t size) {
	if(size % system_page_size() != 0) throw std::runtime_error("size must be multiple of page size");

	int status;
	void* ptr;
	
	FILE* file = ::tmpfile();
	if(file == nullptr) throw std::system_error(errno, std::system_category(), "ring allocator tmpfile failed");
	
	int fd = ::fileno(file);
	
	status = ::ftruncate(fd, size);
	if(status != 0) throw std::system_error(errno, std::system_category(), "ring allocator ftruncate failed");
		
	void* base = ::mmap(nullptr, size * 2, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if(base == MAP_FAILED) throw std::system_error(errno, std::system_category(), "ring allocator mmap failed (whole region)");

	ptr = ::mmap(base, size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED) throw std::system_error(errno, std::system_category(), "ring allocator mmap failed (base)");

	ptr = ::mmap(advance_raw_ptr(base, size), size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED) throw std::system_error(errno, std::system_category(), "ring allocator mmap failed (mirror)");
	
	status = ::close(fd);
	if(status != 0) throw std::system_error(errno, std::system_category(), "ring allocator close failed");
	
	return base;
}


void detail::ring_allocator_base::raw_deallocate(void* base, std::size_t size) {
	::munmap(base, size * 2);
}

}

#endif
