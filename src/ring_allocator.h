#ifndef MF_RING_ALLOCATOR_H_
#define MF_RING_ALLOCATOR_H_

#include <exception>
#include <cassert>
#include "utility/memory.h"

namespace mf {

namespace detail {
	class ring_allocator_base {
	public:
		void* raw_allocate(std::size_t size);
		void raw_deallocate(void* ptr, std::size_t size);
	};
}


template<typename T>
class ring_allocator : private detail::ring_allocator_base {
public:
	using value_type = T;
	using pointer = T*;

	pointer allocate(std::size_t n) {
		return reinterpret_cast<pointer>(raw_allocate(n * sizeof(T)));	
	}
	
	void deallocate(pointer ptr, std::size_t n) {
		raw_deallocate(reinterpret_cast<void*>(ptr), n * sizeof(T));
	}
	
	friend bool operator==(const ring_allocator&, const ring_allocator&) noexcept { return true; }
	friend bool operator!=(const ring_allocator&, const ring_allocator&) noexcept { return false; }
};

}

#endif
