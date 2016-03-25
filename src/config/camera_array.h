#ifndef MF_CONFIG_CAMERA_ARRAY_H_
#define MF_CONFIG_CAMERA_ARRAY_H_

#include "../camera/projection_camera.h"
#include <string>
#include <map>

namespace mf {

class camera_array {
private:
	std::map<std::string, projection_camera> cameras_;
	
	static projection_camera read_camera_(std::istream& str);

public:
	explicit camera_array(const std::string& filename);
	
	bool has(const std::string&) const;
	const projection_camera& operator[](const std::string&) const;
};
	
}

#endif
