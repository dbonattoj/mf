#ifndef MF_UTILITY_STRING_H_
#define MF_UTILITY_STRING_H_

#include <string>
#include <vector>

namespace mf {

std::string file_name_extension(const std::string& filename);
std::string replace_file_name_extension(const std::string& filename, const std::string& ext);
std::string remove_file_name_extension(const std::string& filename);

template<typename T> std::string to_string(const T&);
template<typename T> T from_string(const std::string&);

std::vector<std::string> explode(char separator, const std::string&);
std::string implode(char separator, const std::vector<std::string>&);

template<typename T>
std::vector<T> explode_from_string(char separator, const std::string&);

template<typename T>
std::string implode_to_string(char separator, const std::vector<T>&);

std::string to_lower(const std::string&);
std::string to_upper(const std::string&);

}

#include "string.tcc"

#endif
