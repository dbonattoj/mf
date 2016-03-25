#ifndef MF_RING_ALLOCATOR_H_
#define MF_RING_ALLOCATOR_H_

#include <exception>
#include <cassert>
#include "utility/memory.h"

namespace mf {

class raw_ring_allocator {
public:
	void* raw_allocate(std::size_t size, std::size_t align = 1);
	void raw_deallocate(void* ptr, std::size_t size);
};


template<typename T>
class ring_allocator : private raw_ring_allocator {
public:
	using value_type = T;
	using pointer = T*;

	pointer allocate(std::size_t n) {
		void* raw_ptr = raw_allocate(n * sizeof(T), alignof(T));
		return reinterpret_cast<pointer>(raw_ptr);	
	}
	
	void deallocate(pointer ptr, std::size_t n) {
		void* raw_ptr = reinterpret_cast<void*>(ptr);
		raw_deallocate(raw_ptr, n * sizeof(T));
	}
	
	friend bool operator==(const ring_allocator&, const ring_allocator&) noexcept { return true; }
	friend bool operator!=(const ring_allocator&, const ring_allocator&) noexcept { return false; }
};

}

#endif
