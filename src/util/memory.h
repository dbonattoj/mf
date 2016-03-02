#ifndef MF_MEMORY_H_
#define MF_MEMORY_H_

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

}

#endif