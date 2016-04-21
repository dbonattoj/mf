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
		([&](auto... args) { \
			::mf::detail::debug_print( \
				tag, \
				MF_DEBUG_HEADER("(" #__VA_ARGS__ ") = ", ", "), \
				args... \
			); \
		})(__VA_ARGS__)

	#define MF_DEBUG_EXPR(...) MF_DEBUG_EXPR_T("", __VA_ARGS__)

	#define MF_DEBUG_BACKTRACE(caption) \
		::mf::detail::debug_print_backtrace( \
			MF_DEBUG_HEADER(caption "\nbacktrace:", ""), \
			::mf::detail::debug_get_backtrace())

#else

	#define MF_DEBUG_T(...) ((void)0)
	#define MF_DEBUG(...) ((void)0)
	#define MF_DEBUG_EXPR_T(...) ((void)0)
	#define MF_DEBUG_EXPR(...) ((void)0)
	#define MF_DEBUG_BACKTRACE() ((void)0)

#endif

#include <set>
#include <string>

namespace mf {

enum class debug_mode {
	inactive,
	cerr,
	file
};

void set_debug_mode(debug_mode);

void set_no_debug_filter();
void set_debug_filter(const std::set<std::string>& tags);

void initialize_debug();

}

#include "debug.tcc"

#endif
