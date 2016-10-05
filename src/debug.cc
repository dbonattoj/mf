/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "debug.h"

#ifndef NDEBUG

#include "os/os.h"
#include <map>
#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <vector>
#include <cassert>
#include "utility/misc.h"

namespace mf {

namespace {
	std::map<std::thread::id, std::string> thread_colors_;
	std::vector<std::string> available_thread_colors_ = { "34", "31", "32", "35", "36", "33" };
	std::ptrdiff_t next_thread_color_ = 0;
	
	std::mutex mutex_;
	debug_mode mode_ = debug_mode::cerr;
	std::set<std::string> tags_;
}


namespace detail {
	std::string debug_head(const debug_header& header) {
		auto tid = std::this_thread::get_id();
		std::string color = debug_thread_color();	

		std::string header_beg, body_beg;

		if(mode_ == debug_mode::cerr||1) {
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
		if(mode_ == debug_mode::file&&0) return "\n";	
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
	
	std::FILE* debug_stream(const std::string& tag) {
		if(mode_ == debug_mode::file) {
			std::lock_guard<std::mutex> lock(debug_mutex());
			static std::map<std::string, std::FILE*> files = {};
			if(files.find(tag) == files.end()) {
				std::string filename = "debug_" + tag + ".txt";
				files[tag] = std::fopen(filename.c_str(), "w");
				std::string sep(50, '#');
				std::fprintf(files[tag], "%s\n%s\n%s\n", sep.c_str(), sep.c_str(), sep.c_str());
			}
			return files.at(tag);
		} else if(mode_ == debug_mode::cerr) {
			return stderr;
		} else {
			return nullptr;
		}
	}
	
	bool debug_test_filter(const std::string& tag) {
		std::lock_guard<std::mutex> lock(debug_mutex());
		if(tags_.empty()) return true;
		else return (tags_.find(tag) != tags_.end());
	}
		
	void debug_print_backtrace(const debug_header& header, const std::string& bt) {
		std::FILE* output = debug_stream("");
		if(! output) return;
		
		std::string head = debug_head(header);
		std::string tail = debug_tail();

		std::lock_guard<std::mutex> lock(debug_mutex());	
			
		std::fprintf(output, "%sCall Stack Backtrace:\n%s%s", head.c_str(), bt.c_str(), tail.c_str());
	}
}

namespace detail {
	bool random_sleep_enabled_ = true;
}

void set_random_sleep_enabled(bool enabled) {
	detail::random_sleep_enabled_ = enabled;
}

void set_debug_mode(debug_mode mode) {
	mode_ = mode;
}

void set_no_debug_filter() {
	std::lock_guard<std::mutex> lock(detail::debug_mutex());
	tags_.clear();
}

void set_debug_filter(const std::set<std::string>& tags) {
	std::lock_guard<std::mutex> lock(detail::debug_mutex());
	tags_ = tags;
}


void initialize_debug() { }

void random_sleep() {
	if(! detail::random_sleep_enabled_) return;
	int r = randint<int>(0, std::numeric_limits<int>::max());
	int r1 = r % 10;	
	if(r1 < 4) return;
	else if(r1 < 6) ::usleep(r % 500);
	else ::usleep(10000 + r%2000);
}


}

#else

namespace mf {
	
namespace detail {
	bool random_sleep_enabled_ = false;
}

void set_random_sleep_enabled(bool) { }
void set_debug_mode(debug_mode) { }
void set_no_debug_filter() { }
void set_debug_filter(const std::set<std::string>&) { }
void initialize_debug() { }
void random_sleep() { }

}

#endif
