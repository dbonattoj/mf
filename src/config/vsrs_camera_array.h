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

/// Reader of camera array format used by VSRS.
/** File contains array of names projection cameras, defined using their extrinsic and intrinsic matrices, in plain
 ** text format. Some camera array files contain additional fourth `0 0 0 1` row of extrinsic matrix, while some omit
 ** it. `fourth_extrinsic_row` must be set accordingly. */
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

// TODO automatic fourth_extrinsic_row_
// TODO no depth_projection_parameters here

}

#endif
