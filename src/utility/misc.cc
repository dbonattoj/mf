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

#include "misc.h"
#include <string>
#include <sstream>
#include <cstdio>
#include <thread>
#include <chrono>

namespace mf {


bool file_exists(const std::string& filepath) {
	auto handle = std::fopen(filepath.c_str(), "r");
	if(handle) {
		std::fclose(handle);
		return true;
	} else {
		return false;
	}
}


default_random_engine& random_engine() {
	static default_random_engine engine = default_random_engine(
		std::chrono::system_clock::now().time_since_epoch().count()
	);
	return engine;
}


void sleep(unsigned int seconds) {
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
}


void sleep_ms(unsigned int milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void sleep_us(unsigned int microseconds) {
	std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}


}
