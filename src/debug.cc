#ifndef NDEBUG

#include "utility/os.h"
#include "debug.h"
#include <map>
#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <vector>
#include <cassert>

#ifdef MF_OS_LINUX
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#endif

namespace mf {

namespace {
	std::map<std::thread::id, std::string> thread_colors_;
	std::vector<std::string> available_thread_colors_ = { "34", "31", "32", "35", "36", "33" };
	std::ptrdiff_t next_thread_color_ = 0;
	
	std::mutex mutex_;
	debug_mode mode_ = debug_mode::file;
	
	std::string filename_ = "debug.txt";
	
	#ifdef MF_OS_LINUX
	[[noreturn]] void signal_handler_(int sig) {
		MF_DEBUG_BACKTRACE("signal, aborting");
		std::fflush(nullptr);
		std::abort();
	}
	
	[[noreturn]] void terminate_handler_() {
		MF_DEBUG_BACKTRACE();
		std::fprintf(stderr, "terminated");
		std::fflush(nullptr);
		std::abort();
	}
	#endif
}


namespace detail {
	std::string debug_head(const debug_header& header) {
		auto tid = std::this_thread::get_id();
		std::string color = debug_thread_color();	

		std::string header_beg, body_beg;

		if(mode_ == debug_mode::cerr) {
			auto tid = std::this_thread::get_id();
			std::string color = debug_thread_color();	
	
			header_beg = "\x1b[" + color + ";1m";
			body_beg = "\x1b[0m" "\x1b[" + color + "m";
		}

		return header_beg + "[" + std::string(header.file) + ":" + std::to_string(header.line) + ", "
			+ std::string(header.func) + "]: \n" + (header.caption.empty() ? "" : (header.caption + "\n"))
			+ body_beg;
	}
	
	std::string debug_tail() {	
		if(mode_ == debug_mode::file) return "\n";	
		else return "\x1b[0m\n";
	}

	std::string debug_thread_color() {
		std::lock_guard<std::mutex> lock(mutex_);
		auto tid = std::this_thread::get_id();
		
		auto it = thread_colors_.find(tid);
		if(it != thread_colors_.end()) {
			return it->second;
		} else {
			std::string col = available_thread_colors_[next_thread_color_];
			next_thread_color_ = (next_thread_color_ + 1) % available_thread_colors_.size();
			thread_colors_[tid] = col;
			return col;
		}
	}
		
	std::mutex& debug_mutex() { return mutex_; }
	
	std::FILE* debug_stream() {
		if(mode_ == debug_mode::file) {
			static std::FILE* file = nullptr;
			if(! file) file = std::fopen(filename_.c_str(), "w");
			return file;
		} else if(mode_ == debug_mode::cerr) {
			return stderr;
		} else {
			return nullptr;
		}
	}
	
	#ifdef MF_OS_LINUX
	debug_backtrace debug_get_backtrace() {
		debug_backtrace bt;
		bt.size = ::backtrace(bt.trace, debug_backtrace::max_size);
		return bt;
	}
	
	void debug_print_backtrace(const debug_header& header, const debug_backtrace& bt) {
		std::FILE* output = debug_stream();
		if(! output) return;
		
		std::string head = debug_head(header);
		std::string tail = debug_tail();

		std::lock_guard<std::mutex> lock(debug_mutex());	
			
		std::fprintf(output, "%s", head.c_str());
		::backtrace_symbols_fd(bt.trace, bt.size, fileno(output));
		std::fprintf(output, "%s", tail.c_str());
	}
	#else
	debug_backtrace debug_get_backtrace() { return debug_backtrace(); }
	void debug_print_backtrace(const debug_header& header, const debug_backtrace&) {
		debug_print(header, "no support for debug backtrace print");
	}
	#endif
}

void set_debug_mode(debug_mode mode) {
	mode_ = mode;
}

void initialize_debug(
) {
	#ifdef MF_OS_LINUX
	//std::set_terminate(&terminate_handler_);
	::signal(SIGSEGV, &signal_handler_);
	::signal(SIGILL, &signal_handler_);
	#endif
}

}

#else


void set_debug_mode(debug_mode) { }
void initialize_debug() { }


#endif
