#ifndef MF_UTILITY_MEMORY_H_
#define MF_UTILITY_MEMORY_H_

#include "../common.h"
#include <cassert>
#include <cstddef>

namespace mf {

/// Get page size of operating system, in bytes.
std::size_t system_page_size();


/// Round `n` up so that `T[n]` has a size that is a multiple of the system page size.
template<typename T>
std::size_t round_up_to_fit_system_page_size(std::size_t n) {
	MF_EXPECTS(system_page_size() % sizeof(T) == 0);
	std::size_t page_capacity = system_page_size() / sizeof(T);
	std::size_t remaining = page_capacity - (n % page_capacity);
	return n + remaining;
}

/// Round number of bytes `len` up to a multiple of the system page size.
inline std::size_t raw_round_up_to_fit_system_page_size(std::size_t len) {
	return round_up_to_fit_system_page_size<byte>(len);
}

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
	void* raw_allocate(std::size_t size, std::size_t align = 1);	
	void raw_deallocate(void* ptr, std::size_t size);
};


/// Ring allocator, allocates ring buffer memory.
/** Allocates given segment of memory, and maps additional same sized segment immediatly after it in virtual memory,
 ** which is mapped to the same allocated segment. In the allocated segment `seg`, `seg[i]` and `seg[i+n]` always
 ** map to the same data. The segment length `n` must be a multiple of the system page size. */
class raw_ring_allocator {
public:
	void* raw_allocate(std::size_t size, std::size_t align = 1);
	void raw_deallocate(void* ptr, std::size_t size);
};


/// Null allocator, allocates virtual memory which is not used.
/** Allocates segment of virtual memory which does not store data. Allocated memory may be read and written, but does
 ** not need to retain values. Used to allocate buffers that will not be used. OS-specific implementation may map
 ** virtual memory pages without allocating physical memory pages, when possible. */
class raw_null_allocator {
public:
	void* raw_allocate(std::size_t size, std::size_t align = 1);
	void raw_deallocate(void* ptr, std::size_t size);
};


/*
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
*/


}

#endif
