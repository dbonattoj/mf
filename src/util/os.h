#ifndef MF_OS_H_
#define MF_OS_H_

#if defined(__APPLE__)
	#define MF_OS_DARWIN
#elif defined(_WIN32)
	#define MF_OS_WINDOWS
	#if defined(_WIN64)
		#define MF_OS_WINDOWS64
	#endif
#elif __linux__
	#define MF_OS_LINUX
#endif

#endif
