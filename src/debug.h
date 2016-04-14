#ifndef MF_DEBUG_H_
#define MF_DEBUG_H_


#ifndef NDEBUG

	#define MF_DEBUG_SOURCE_LOCATION() \
		::mf::detail::source_location { __FILE__, __LINE__, __func__ }
	
	#define MF_DEBUG(...) \
		::mf::detail::debug_print( \
			MF_DEBUG_SOURCE_LOCATION(), \
			__VA_ARGS__)
			
	#define MF_DEBUG_BACKTRACE() \
		::mf::detail::debug_print_backtrace( \
			MF_DEBUG_SOURCE_LOCATION(), \
			::mf::detail::debug_get_backtrace())

	#define MF_DEBUG_EXPR(...)
		[&](auto... args) { \
			::mf::detail::debug_print( \
				MF_DEBUG_SOURCE_LOCATION(), \
				args... \
			); \
		}("(", #__VA_ARGS__, "):\n", __VA_ARGS__)
	// TODO test

#else

	#define MF_DEBUG(...)
	#define MF_DEBUG_BACKTRACE()

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
