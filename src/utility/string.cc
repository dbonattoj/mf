#include "string.h"
#include <string>
#include <sstream>
#include <cstdio>

namespace mf {

std::vector<std::string> explode(char separator, const std::string& str) {
	std::vector<std::string> vec;
	std::string::size_type last_pos = 0;
	std::string::size_type pos = str.find(separator);
	while(pos != std::string::npos) {
		std::string::size_type n = pos - last_pos;
		std::string part = str.substr(last_pos, n);
		vec.push_back(part);
		last_pos = pos + 1;
		pos = str.find(separator, last_pos);
	}
	vec.push_back(str.substr(last_pos));
	return vec;
}


std::string implode(char separator, const std::vector<std::string>& vec) {
	std::ostringstream ostr;
	auto last = vec.end() - 1;
	for(auto it = vec.begin(); it != last; ++it) {
		ostr << *it << separator;
	}
	ostr << vec.back();
	return ostr.str();
}



}
