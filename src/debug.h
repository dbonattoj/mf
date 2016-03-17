#ifndef MF_DEBUG_H_
#define MF_DEBUG_H_

#ifndef NDEBUG
	// debugging is enabled
	#include <iostream>
	#include <thread>
	#include <mutex>

	#define MF_DEBUG(...) ::mf::detail::debug_print(std::cerr, __FILE__, __LINE__, __func__, __VA_ARGS__)

	namespace mf {

	namespace detail {
		inline std::mutex& debug_print_mutex() {
			static std::mutex mut;
			return mut;
		}
	
		inline void debug_print_part(std::ostream& str) { }	

		template<typename First_arg, typename... Args>
		void debug_print_part(std::ostream& str, const First_arg& first, const Args&... args) {
			str << first;
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
	#define MF_DEBUG(...) ;
	
#endif

#endif