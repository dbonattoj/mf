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

#ifndef MF_OS_H_
#define MF_OS_H_

// macros for operating system
#if defined(__APPLE__)
	#define MF_OS_DARWIN
#elif defined(_WIN32)
	#define MF_OS_WINDOWS
	#if defined(_WIN64)
		#define MF_OS_WINDOWS64
	#endif
#elif defined(__linux__)
	#define MF_OS_LINUX
#endif

// macros for compiler
#if defined(__clang__)
	#define MF_COMPILER_CLANG
#elif defined(__GNUC__)
	#define MF_COMPILER_GCC
#elif defined(_MSC_VER)
	#define MF_COMPILER_MSVC
#endif


#endif



