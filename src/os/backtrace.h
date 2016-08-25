#ifndef MF_OS_BACKTRACE_H_
#define MF_OS_BACKTRACE_H_

#include <string>

namespace mf {

/// Generate formatted string with call stack backtrace.
/** Used for debugging. Format of returned string is implementation-defined and depends on OS. Contains backtrace up
 ** to the call to this function. Only works in debug build. */
std::string stack_backtrace_string() noexcept;

#ifdef NDEBUG
inline std::string stack_backtrace_string() noexcept {
	return "backtrace available only for debug build";
}
#endif

}

#endif
