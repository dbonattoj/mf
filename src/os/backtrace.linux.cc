/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
	for(std::ptrdiff_t i = 1; i < size; ++i) // don't include call to this function
		str << trace_strings[i] << '\n';
	
	::free(trace_strings);
	
	return str.str();
}

}

#endif
#endif
