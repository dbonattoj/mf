#include "camera_array.h"
#include <fstream>
#include <string>
#include "../eigen.h"
#include "../geometry/pose.h"
#include <iostream>

namespace mf {

projection_camera camera_array::read_camera_(std::istream& str) {
	str.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	
	Eigen::Matrix4f intrinsic = Eigen::Matrix4f::Zero();
	str >> intrinsic(0, 0) >> intrinsic(0, 1) >> intrinsic(0, 2);
	str >> intrinsic(1, 0) >> intrinsic(1, 1) >> intrinsic(1, 2);
	str >> intrinsic(2, 0) >> intrinsic(2, 1) >> intrinsic(2, 2);
	intrinsic(3, 2) = -1; intrinsic(2, 3) = -1;
	
	float gomi[2]; // ?
	str >> gomi[0] >> gomi[1];

	Eigen::Matrix4f extrinsic = Eigen::Matrix4f::Zero();
	str >> extrinsic(0, 0) >> extrinsic(0, 1) >> extrinsic(0, 2) >> extrinsic(0, 3);
	str >> extrinsic(1, 0) >> extrinsic(1, 1) >> extrinsic(1, 2) >> extrinsic(1, 3);
	str >> extrinsic(2, 0) >> extrinsic(2, 1) >> extrinsic(2, 2) >> extrinsic(2, 3);
	extrinsic(3, 3) = 1;
	
	Eigen::Affine3f extrinsic_affine;
	extrinsic_affine = extrinsic;
		
	return projection_camera(extrinsic_affine, intrinsic);
}


camera_array::camera_array(const std::string& filename) {
	std::ifstream file(filename);
	
	for(;;) {
		file.exceptions(std::ios_base::badbit);	

		std::string name;
		file >> name;
		if(file.eof() || file.fail()) break;
		
		cameras_.insert(std::make_pair(
			name,
			read_camera_(file)
		));
	}
}


bool camera_array::has(const std::string& name) const {
	return (cameras_.find(name) != cameras_.end());
}


const projection_camera& camera_array::operator[](const std::string& name) const {
	return cameras_.at(name);
}


}

