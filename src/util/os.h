#ifndef MF_OS_H_
#define MF_OS_H_

#if defined(__APPLE__)
	#define MF_OS_DARWIN
#elif defined(_WIN32)
	#define MF_OS_WINDOWS
#else
	
#endif

#endif