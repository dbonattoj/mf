#ifndef MF_DEBUG_H_
#define MF_DEBUG_H_


#ifndef NDEBUG

	#define MF_DEBUG_HEADER(caption, separator) \
		::mf::detail::debug_header { caption, separator, __FILE__, __LINE__, __func__ }
	
	#define MF_DEBUG(...) \
		::mf::detail::debug_print( \
			MF_DEBUG_HEADER("", ""), \
			__VA_ARGS__)
			
	#define MF_DEBUG_BACKTRACE(caption) \
		::mf::detail::debug_print_backtrace( \
			MF_DEBUG_HEADER(caption "\nbacktrace:", ""), \
			::mf::detail::debug_get_backtrace())

	#define MF_DEBUG_EXPR(...) \
		([&](auto... args) { \
			::mf::detail::debug_print( \
				MF_DEBUG_HEADER("(" #__VA_ARGS__ ") = ", ", "), \
				args... \
			); \
		})(__VA_ARGS__)

#else

	#define MF_DEBUG(...) ((void)0)
	#define MF_DEBUG_BACKTRACE() ((void)0)
	#define MF_DEBUG_EXPR(...) ((void)0)

#endif


namespace mf {

enum class debug_mode {
	inactive,
	cerr,
	file
};

void set_debug_mode(debug_mode);

void initialize_debug();

}

#include "debug.tcc"

#endif
