#ifndef MF_DEBUG_H_
#define MF_DEBUG_H_

#ifndef NDEBUG
	// debugging is enabled
	#include <fstream>
	#include <thread>
	#include <mutex>

	#define MF_DEBUG(...) ::mf::detail::debug_print( \
		::mf::detail::debug_ostream(), \
		__FILE__, \
		__LINE__, \
		__func__, \
		__VA_ARGS__)

	namespace mf {

	namespace detail {
		inline std::mutex& debug_print_mutex() {
			static std::mutex mut;
			return mut;
		}
		
		inline std::ostream& debug_ostream() {
			static std::ofstream file("debug.txt");
			return file;
		}
	
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
			std::lock_guard<std::mutex> lock(debug_print_mutex());			
			auto tid = std::this_thread::get_id();
			str << "\x1b[34;1m[" << file << ':' << line << ", " << func << ", thread " << tid << "]:\n\x1b[0m";
			debug_print_part(str, args...);
			str << "\n\n";	
		}
	}
	
	}

#else
	// debugging is disabled	
	#define MF_DEBUG(...) 
	
#endif

#endif
