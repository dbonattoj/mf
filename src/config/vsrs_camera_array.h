#ifndef MF_VSRS_CAMERA_ARRAY_H_
#define MF_VSRS_CAMERA_ARRAY_H_

#include "../common.h"
#include "../geometry/depth_projection_parameters.h"
#include "../camera/projection_camera.h"
#include "../ndarray/ndcoord.h"
#include <string>
#include <map>
#include <cstdint>

namespace mf {

class vsrs_camera_array {
private:
	std::map<std::string, projection_camera> cameras_;
	bool fourth_extrinsic_row_ = false;
	
	projection_camera read_camera_(std::istream& str, const depth_projection_parameters&, const ndsize<2>&);

public:
	vsrs_camera_array(const std::string& filename, const depth_projection_parameters&, const ndsize<2>&, bool fourth_extrinsic_row = false);
		
	bool has(const std::string&) const;
	const projection_camera& operator[](const std::string&) const;
};
	
}

#endif
