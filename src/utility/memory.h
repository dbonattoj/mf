#ifndef MF_UTILITY_MEMORY_H_
#define MF_UTILITY_MEMORY_H_

#include <cassert>
#include <cstddef>

namespace mf {

std::size_t system_page_size();

template<typename T>
std::size_t round_up_to_fit_system_page_size(std::size_t n) {
	assert(system_page_size() % sizeof(T) == 0);
	std::size_t page_capacity = system_page_size() / sizeof(T);
	std::size_t remaining = page_capacity - (n % page_capacity);
	return n + remaining;
}

enum class memory_usage_advice {
	normal,
	sequential,
	random
};

void set_memory_usage_advice(void*, std::size_t, memory_usage_advice);


/// Raw allocator which allocates given number of bytes.
class raw_allocator {
public:
	void* raw_allocate(std::size_t size, std::size_t align = 1);	
	void raw_deallocate(void* ptr, std::size_t size);
};


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
