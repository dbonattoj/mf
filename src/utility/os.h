#ifndef MF_UTILITY_OS_H_
#define MF_UTILITY_OS_H_

// macros for OS
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



