#ifndef MF_DEBUG_H_
#define MF_DEBUG_H_

namespace mf {

enum class debug_mode {
	inactive,
	cerr,
	file
};

}

#ifndef NDEBUG
	// debugging is enabled
	#include <thread>
	#include <mutex>
	#include <ostream>

	#define MF_DEBUG(...) \
		if(::mf::detail::debug_is_enabled()) \
			::mf::detail::debug_print( \
				::mf::detail::debug_ostream(), \
				__FILE__, \
				__LINE__, \
				__func__, \
				__VA_ARGS__)

	namespace mf {
		
	namespace detail {		
		std::string debug_thread_color();	
		std::mutex& debug_mutex();
		std::ostream& debug_ostream();
		bool debug_is_enabled();
	
		inline void debug_print_part(std::ostream& str) { }	
		
		template<typename First_arg, typename... Args>
		auto debug_print_part(std::ostream& str, const First_arg& first, const Args&... args)
			-> decltype(first.debug_print(str), void());
		
		template<typename First_arg, typename... Args>
		auto debug_print_part(std::ostream& str, const First_arg& first, const Args&... args)
			-> decltype(str << first, void())
		{
			str << first;
			debug_print_part(str, args...);
		}
		
		template<typename First_arg, typename... Args>
		auto debug_print_part(std::ostream& str, const First_arg& first, const Args&... args)
			-> decltype(first.debug_print(str), void())
		{
			first.debug_print(str);
			debug_print_part(str, args...);
		}
		
		template<typename... Args>
		void debug_print(std::ostream& str, const char* file, int line, const char* func, const Args&... args) {
			auto tid = std::this_thread::get_id();
			std::string color = debug_thread_color();			
			std::lock_guard<std::mutex> lock(debug_mutex());
			
			str << "\x1b[37m[" << file << ':' << line << ", " << func << ", thread " << tid << "]:\x1b[0m \n";
			str << "\x1b[" + color + "m";
			debug_print_part(str, args...);
			str << "\x1b[0m\n" << std::endl;
		}
	}
	
	void set_debug_mode(debug_mode);
	
	}

#else
	// debugging is disabled	
	#define MF_DEBUG(...)
	
	inline void set_debug_mode(debug_mode) { }
	
#endif

#endif
