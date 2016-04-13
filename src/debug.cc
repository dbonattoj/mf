#ifndef MF_DEBUG_H_
#define MF_DEBUG_H_

#ifndef NDEBUG

#include "debug.h"
#include <map>
#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <vector>

namespace mf {

namespace {
	std::map<std::thread::id, std::string> thread_colors_;
	std::vector<std::string> available_thread_colors_ = { "34", "31", "32", "35", "36", "33" };
	std::ptrdiff_t next_thread_color_ = 0;
	
	std::mutex mutex_;
	bool active_ = true;
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
	
	bool debug_is_active() { return active_; }
	void set_debug_active(bool active) { active_ = active; }
	
	std::mutex& debug_mutex() { return mutex_; }
	
	std::ostream& debug_ostream() {
		static std::ofstream file("debug.txt");
		return file;
	}
}

}

#endif

#endif
