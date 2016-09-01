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
#include "debug.h"
#include "exceptions.h"

#define MF_VERIFY_ASSERTIONS 1

#define MF_STRINGIZE_(X) #X
#define MF_STRINGIZE(X) MF_STRINGIZE_(X)

#define MF_GET_NARG_MACRO_2(_1, _2, NAME, ...) NAME


#ifdef MF_DEBUG_BUILD
	#define MF_ASSERT_CRIT_MSG_(__condition__, __msg__) \
		if(! (__condition__)) { \
			throw ::mf::failed_assertion(__msg__ " at " __FILE__ ":" MF_STRINGIZE(__LINE__)); \
		}

	#define MF_ASSERT_MSG_(__condition__, __msg__) \
		if(! (__condition__)) { \
			throw ::mf::failed_assertion(__msg__ " at " __FILE__ ":" MF_STRINGIZE(__LINE__)); \
		}
#else 
	#define MF_ASSERT_CRIT_MSG_(__condition__, __msg__) ((void)0)
	
	#if MF_VERIFY_ASSERTIONS
		#define MF_ASSERT_MSG_(__condition__, __msg__) \
			if(! (__condition__)) throw ::mf::failed_assertion(__msg__ " at " __FILE__ ":" MF_STRINGIZE(__LINE__))
	#else
		#define MF_ASSERT_MSG_(__condition__, __msg__) ((void)0)
	#endif
#endif


//#define MF_MACRO_DEPRECATED _Pragma("GCC warning \"macro is deprecated\"");
#define MF_MACRO_DEPRECATED

#define MF_ASSERT_(__condition__) MF_ASSERT_MSG_(__condition__, "`" #__condition__ "`")
#define MF_ASSERT_CRIT_(__condition__) MF_ASSERT_CRIT_MSG_(__condition__, "`" #__condition__ "`")

#define MF_ASSERT(...) MF_GET_NARG_MACRO_2(__VA_ARGS__, MF_ASSERT_MSG_, MF_ASSERT_, IGNORE)(__VA_ARGS__)
#define MF_ASSERT_CRIT(...) MF_GET_NARG_MACRO_2(__VA_ARGS__, MF_ASSERT_CRIT_MSG_, MF_ASSERT_CRIT_, IGNORE)(__VA_ARGS__)

#define MF_EXPECTS MF_MACRO_DEPRECATED MF_ASSERT
#define MF_ENSURES MF_MACRO_DEPRECATED MF_ASSERT

#define MF_EXPECTS_CRIT MF_MACRO_DEPRECATED MF_ASSERT_CRIT
#define MF_ENSURES_CRIT MF_MACRO_DEPRECATED MF_ASSERT_CRIT


#define Assert MF_ASSERT
#define Expects MF_EXPECTS
#define Ensures MF_ENSURES
#define Assert_crit MF_ASSERT_CRIT
#define Expects_crit MF_EXPECTS_CRIT
#define Ensures_crit MF_ENSURES_CRIT

#define MF_ASSERT_MSG MF_ASSERT
#define MF_EXPECTS_MSG MF_EXPECTS
#define MF_ENSURES_MSG MF_ENSURES


namespace mf {


/// Real number type.
using real = double;


/// Discrete time unit type.
using time_unit = std::ptrdiff_t;


/// Single byte type.
using byte = std::uint8_t;


}

#endif
