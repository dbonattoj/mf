#ifndef MF_RAW_ALLOCATOR_H_
#define MF_RAW_ALLOCATOR_H_

#include <cstddef>

namespace mf {

/// Raw allocator which allocates given number of bytes.
class raw_allocator {
public:
	void* raw_allocate(std::size_t size, std::size_t align = 1);	
	void raw_deallocate(void* ptr, std::size_t size);
};

}

#endif
