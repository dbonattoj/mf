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
