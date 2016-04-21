#ifndef NDEBUG

#include <thread>
#include <mutex>
#include <cstdio>
#include <ostream>
#include <sstream>
#include <string>

namespace mf {
	
namespace detail {					
	struct debug_header {
		std::string caption;
		std::string separator;

		const char* file;
		int line;
		const char* func;
	};
	
	struct debug_backtrace {
		constexpr static std::size_t max_size = 20;
	
		void* trace[max_size];
		std::size_t size;
	};	
	
	std::string debug_thread_color();	
	std::mutex& debug_mutex();
	std::FILE* debug_stream();
	debug_backtrace debug_get_backtrace();

	inline void debug_print_part(std::ostream& str, const std::string& sep) { }	
	
	template<typename First_arg, typename... Args>
	auto debug_print_part(std::ostringstream& str, const std::string& sep, const First_arg& first, const Args&... args)
		-> decltype(first.debug_print(str), void());
	
	template<typename First_arg, typename... Args>
	auto debug_print_part(std::ostringstream& str, const std::string& sep, const First_arg& first, const Args&... args)
		-> decltype(str << first, void())
	{
		str << first;
		if(sizeof...(Args) > 0) str << sep;
		debug_print_part(str, sep, args...);
	}
	
	template<typename First_arg, typename... Args>
	auto debug_print_part(std::ostringstream& str, const std::string& sep, const First_arg& first, const Args&... args)
		-> decltype(first.debug_print(str), void())
	{
		first.debug_print(str);
		if(sizeof...(Args) > 0) str << sep;
		debug_print_part(str, sep, args...);
	}
	
	std::string debug_head(const debug_header&);
	std::string debug_tail();
	bool debug_test_filter(const std::string& tag);
	
	template<typename... Args>
	void debug_print(const std::string& tag, const debug_header& header, const Args&... args) {
		if(! debug_test_filter(tag)) return;
		
		std::FILE* output = debug_stream();
		if(! output) return;
		
		std::string head = debug_head(header);
		std::string tail = debug_tail();

		std::ostringstream str;			
		debug_print_part(str, header.separator, args...);

		std::lock_guard<std::mutex> lock(debug_mutex());
		std::fprintf(output, "%s%s%s\n", head.c_str(), str.str().c_str(), tail.c_str());
	}
	
	void debug_print_backtrace(const debug_header&, const debug_backtrace&);
}

}

#endif
