#include "os.h"
#ifdef MF_OS_WINDOWS

#include "memory.h"
#include <windows.h>

namespace mf {

std::size_t system_page_size() {
	static std::size_t page_size = 0;
		if(page_size == 0) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		page_size = si.dwPageSize;
	}
	return page_size;
}

void set_memory_usage_advice(void* buf, std::size_t len, memory_usage_advice adv) {
	// Not available on Windows
	return;
}

}

#endif
