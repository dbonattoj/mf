#ifndef NDEBUG

#include <thread>
#include <mutex>
#include <cstdio>
#include <ostream>
#include <sstream>

namespace mf {
	
namespace detail {					
	struct source_location {
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

	inline void debug_print_part(std::ostream& str) { }	
	
	template<typename First_arg, typename... Args>
	auto debug_print_part(std::ostringstream& str, const First_arg& first, const Args&... args)
		-> decltype(first.debug_print(str), void());
	
	template<typename First_arg, typename... Args>
	auto debug_print_part(std::ostringstream& str, const First_arg& first, const Args&... args)
		-> decltype(str << first, void())
	{
		str << first;
		debug_print_part(str, args...);
	}
	
	template<typename First_arg, typename... Args>
	auto debug_print_part(std::ostringstream& str, const First_arg& first, const Args&... args)
		-> decltype(first.debug_print(str), void())
	{
		first.debug_print(str);
		debug_print_part(str, args...);
	}
	
	std::string debug_head(const source_location& loc);
	std::string debug_tail();
	
	template<typename... Args>
	void debug_print(const source_location& loc, const Args&... args) {		
		std::FILE* output = debug_stream();
		if(! output) return;
		
		std::string head = debug_head(loc);
		std::string tail = debug_tail();

		std::ostringstream str;			
		debug_print_part(str, args...);

		std::lock_guard<std::mutex> lock(debug_mutex());			
		std::fprintf(output, "%s%s%s\n", head.c_str(), str.str().c_str(), tail.c_str());
	}
	
	void debug_print_backtrace(const source_location&, const debug_backtrace&);
}

}

#endif
