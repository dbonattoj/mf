#ifndef NDEBUG

#include "debug.h"
#include <map>
#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <vector>
#include <cassert>

namespace mf {

namespace {
	std::map<std::thread::id, std::string> thread_colors_;
	std::vector<std::string> available_thread_colors_ = { "34", "31", "32", "35", "36", "33" };
	std::ptrdiff_t next_thread_color_ = 0;
	
	std::mutex mutex_;
	debug_mode mode_ = debug_mode::cerr;
}

namespace detail {
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
	
	std::ostream& debug_ostream() {
		assert(mode_ != debug_mode::inactive);
		if(mode_ == debug_mode::file) {
			static std::ofstream file("debug.txt");
			return file;
		} else if(mode_ == debug_mode::cerr) {
			return std::cerr;
		}
	}
	
	bool debug_is_enabled() {
		return (mode_ != debug_mode::inactive);
	}
}

void set_debug_mode(debug_mode mode) {
	mode_ = mode;
}

}

#endif
