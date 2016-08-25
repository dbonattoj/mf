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

#ifndef NDEBUG

#include <thread>
#include <mutex>
#include <cstdio>
#include <ostream>
#include <sstream>
#include <string>

namespace mf {
	
namespace detail {					
	struct debug_header {
		std::string caption;
		std::string separator;

		const char* file;
		int line;
		const char* func;
	};
	
	std::string debug_thread_color();	
	std::mutex& debug_mutex();
	std::FILE* debug_stream();

	inline void debug_print_part(std::ostream& str, const std::string& sep) { }	
	
	template<typename First_arg, typename... Args>
	auto debug_print_part(std::ostringstream& str, const std::string& sep, const First_arg& first, const Args&... args)
		-> decltype(first.debug_print(str), void());
	
	template<typename First_arg, typename... Args>
	auto debug_print_part(std::ostringstream& str, const std::string& sep, const First_arg& first, const Args&... args)
		-> decltype(str << first, void())
	{
		str << first;
		if(sizeof...(Args) > 0) str << sep;
		debug_print_part(str, sep, args...);
	}
	
	template<typename First_arg, typename... Args>
	auto debug_print_part(std::ostringstream& str, const std::string& sep, const First_arg& first, const Args&... args)
		-> decltype(first.debug_print(str), void())
	{
		first.debug_print(str);
		if(sizeof...(Args) > 0) str << sep;
		debug_print_part(str, sep, args...);
	}
	
	std::string debug_head(const debug_header&);
	std::string debug_tail();
	bool debug_test_filter(const std::string& tag);
	
	template<typename... Args>
	void debug_print(const std::string& tag, const debug_header& header, const Args&... args) {
		if(! debug_test_filter(tag)) return;
		
		std::FILE* output = debug_stream();
		if(! output) return;
		
		std::string head = debug_head(header);
		std::string tail = debug_tail();

		std::ostringstream str;			
		debug_print_part(str, header.separator, args...);

		std::lock_guard<std::mutex> lock(debug_mutex());
		std::fprintf(output, "%s%s%s\n", head.c_str(), str.str().c_str(), tail.c_str());
	}
	
	void debug_print_backtrace(const debug_header&, const std::string& bt);
}

}

#endif
