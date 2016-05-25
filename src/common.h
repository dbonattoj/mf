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

#ifndef MF_COMMON_H_
#define MF_COMMON_H_

#include <cstddef>
#include <cassert>
#include <exception>
#include <stdexcept>

#include <ostream>

#include "ndarray/ndspan.h"
#include "debug.h"
#include "exceptions.h"


#ifndef NDEBUG

	#define MF_ASSERT_MSG(condition, msg) \
		if(!(condition)) { \
			MF_DEBUG_BACKTRACE("assertion failed: " msg); \
			std::abort(); \
		}

	#define MF_ASSERT(condition) MF_ASSERT_MSG(condition, "`" #condition "`")
	
	#define MF_EXPECTS_MSG(condition, msg) MF_ASSERT_MSG(condition, "precondition: " msg)
	#define MF_EXPECTS(condition) MF_EXPECTS_MSG(condition, "`" #condition "`")
		
	#define MF_ENSURES_MSG(condition, msg) MF_ASSERT_MSG(condition, "postcondition: " msg)
	#define MF_ENSURES(condition) MF_EXPECTS_MSG(condition, "`" #condition "`")

#else

	#define MF_ASSERT_MSG(condition, msg) ((void)0)
	#define MF_ASSERT(condition) ((void)0)
	#define MF_EXPECTS(condition) ((void)0)
	#define MF_EXPECTS_MSG(condition, msg) ((void)0)
	#define MF_ENSURES(condition) ((void)0)
	#define MF_ENSURES_MSG(condition, msg) ((void)0)
	
#endif


#define MF_STATIC_ASSERT(condition) static_assert(condition, #condition)
#define MF_STATIC_ASSERT_MSG(condition, msg) static_assert(condition, msg)



namespace mf {


/// Real number type.
using real = float;


/// Discrete time unit type.
using time_unit = std::ptrdiff_t;


/// Single byte type.
using byte = std::uint8_t;


/// One-dimensional time span.
/** Derived from `ndspan<1, time_unit>.` */
class time_span : public ndspan<1, time_unit> {
	using base = ndspan<1, time_unit>;
	
public:
	time_span() = default;
	time_span(const base& span) : base(span) { }
	time_span(time_unit start, time_unit end) :
		base{start, end} { }
	
	time_unit start_time() const { return base::start_pos().front(); }
	time_unit end_time() const { return base::end_pos().front(); }
	time_unit duration() const { return base::size(); }
};


inline std::ostream& operator<<(std::ostream& str, const time_span& span) {
	str << '[' << span.start_time() << ", " << span.end_time() << '[';
	return str;
}


}

#endif
