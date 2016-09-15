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

#ifndef MF_UTILITY_UID_POOL_H_
#define MF_UTILITY_UID_POOL_H_

#include <map>
#include <cstdint>
#include <string>
#include <memory>
#include <utility>

namespace mf {

/// Helper class which assigns unique IDs to objects based on their address.
class uid_pool {
private:
	std::map<std::uintptr_t, std::string> uids_;

public:
	void clear() {
		uids_.clear();
	}

	template<typename T>
	const std::string& uid(const T& obj, const std::string& prefix = "") {
		std::uintptr_t address = reinterpret_cast<std::uintptr_t>(std::addressof(obj));
		auto it = uids_.find(address);
		if(it != uids_.end()) {
			return it->second;
		} else {
			std::size_t index = uids_.size();
			std::string uid = prefix + std::to_string(index);
			auto res = uids_.emplace(address, std::move(uid));
			return res.first->second;
		}
	}
};

}

#endif
