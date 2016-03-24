#include "misc.h"
#include <string>
#include <sstream>
#include <cstdio>

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

}
