#ifndef MF_TANIMOTO_CAMERA_ARRAY_H_
#define MF_TANIMOTO_CAMERA_ARRAY_H_

#include "../common.h"
#include "../geometry/depth_projection_parameters.h"
#include "../camera/projection_camera.h"
#include "../ndarray/ndcoord.h"
#include <string>
#include <map>
#include <cstdint>

namespace mf {

class tanimoto_camera_array {
private:
	std::map<std::string, projection_camera> cameras_;
	
	static projection_camera read_camera_(std::istream& str, const depth_projection_parameters&);

public:
	tanimoto_camera_array(const std::string& filename, const depth_projection_parameters&);
	
	bool has(const std::string&) const;
	const projection_camera& operator[](const std::string&) const;
};
	
}

#endif
