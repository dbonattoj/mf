#include "os.h"
#ifndef NDEBUG
#ifdef MF_OS_LINUX

#include "backtrace.h"
#include <execinfo.h>
#include <sstream>

namespace mf {

std::string stack_backtrace_string() noexcept {
	constexpr std::size_t max_size = 64;
	
	void* trace[max_size];
	int size = ::backtrace(trace, max_size);
	
	char** trace_strings = ::backtrace_symbols(trace, size);
	if(trace_strings == nullptr) return "could not get backtrace";
	
	std::ostringstream str;
	for(std::ptrdiff_t i = 0; i < size; ++i)
		str << trace_strings[i] << '\n';
	
	::free(trace_strings);
	
	return str.str();
}

}

#endif
#endif
