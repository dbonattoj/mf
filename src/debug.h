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

#ifndef MF_DEBUG_H_
#define MF_DEBUG_H_

#ifndef NDEBUG

	#define MF_DEBUG_HEADER(caption, separator) \
		::mf::detail::debug_header { caption, separator, __FILE__, __LINE__, __func__ }
	
	#define MF_DEBUG_T(tag, ...) \
		::mf::detail::debug_print( \
			tag, \
			MF_DEBUG_HEADER("", ""), \
			__VA_ARGS__)
	
	#define MF_DEBUG(...) MF_DEBUG_T("", __VA_ARGS__)	

	#define MF_DEBUG_EXPR_T(tag, ...) \
		([&](const ::mf::detail::debug_header& header, auto... args) { \
			::mf::detail::debug_print(tag, header, args...); \
		})(MF_DEBUG_HEADER("(" #__VA_ARGS__ ") = ", ", "), __VA_ARGS__)

	#define MF_DEBUG_EXPR(...) MF_DEBUG_EXPR_T("", __VA_ARGS__)

	#define MF_DEBUG_BACKTRACE(caption) \
		::mf::detail::debug_print_backtrace( \
			MF_DEBUG_HEADER(caption "\nbacktrace:", ""), \
			::mf::detail::debug_get_backtrace())
	
	#define MF_RAND_SLEEP \
		random_sleep()

#else

	#define MF_DEBUG_T(...) ((void)0)
	#define MF_DEBUG(...) ((void)0)
	#define MF_DEBUG_EXPR_T(...) ((void)0)
	#define MF_DEBUG_EXPR(...) ((void)0)
	#define MF_DEBUG_BACKTRACE(...) ((void)0)
	#define MF_RAND_SLEEP ((void)0)

#endif

#include <set>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <limits>
#include "utility/misc.h"

namespace mf {

namespace detail {
	extern bool random_sleep_enabled_;
}

enum class debug_mode {
	inactive,
	cerr,
	file
};

void set_debug_mode(debug_mode);

void set_no_debug_filter();
void set_debug_filter(const std::set<std::string>& tags);
void set_random_sleep_enabled(bool);

void initialize_debug();

inline void random_sleep() {
	if(! detail::random_sleep_enabled_) return;
	int r = randint<int>(0, std::numeric_limits<int>::max());
	int r1 = r % 10;	
	if(r1 < 4) return;
	else if(r1 < 6) ::usleep(r % 500);
	else ::usleep(10000 + r%2000);
}

}

#include "debug.tcc"

#endif
