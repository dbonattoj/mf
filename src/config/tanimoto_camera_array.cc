#include "camera_array.h"
#include <fstream>
#include <string>
#include "../eigen.h"
#include "../geometry/pose.h"
#include <iostream>

namespace mf {

projection_camera tanimoto_camera_array::read_camera_(std::istream& str, const depth_projection_parameters& dparam) {
	str.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	
	Eigen_mat3 intrinsic = Eigen_mat3::Zero();
	str >> intrinsic(0, 0) >> intrinsic(0, 1) >> intrinsic(0, 2);
	str >> intrinsic(1, 0) >> intrinsic(1, 1) >> intrinsic(1, 2);
	str >> intrinsic(2, 0) >> intrinsic(2, 1) >> intrinsic(2, 2);
	
	float gomi[2]; // ?
	str >> gomi[0] >> gomi[1];

	Eigen_mat4 extrinsic = Eigen_mat4::Zero();
	str >> extrinsic(0, 0) >> extrinsic(0, 1) >> extrinsic(0, 2) >> extrinsic(0, 3);
	str >> extrinsic(1, 0) >> extrinsic(1, 1) >> extrinsic(1, 2) >> extrinsic(1, 3);
	str >> extrinsic(2, 0) >> extrinsic(2, 1) >> extrinsic(2, 2) >> extrinsic(2, 3);
	extrinsic(3, 3) = 1.0;
	
	Eigen::Affine3f extrinsic_affine;
	extrinsic_affine = extrinsic;
		
	return projection_camera(extrinsic_affine, intrinsic, dparam);
}


tanimoto_camera_array::tanimoto_camera_array(const std::string& filename, const depth_projection_parameters& dparam) {
	std::ifstream file(filename);
	
	for(;;) {
		file.exceptions(std::ios_base::badbit);	

		std::string name;
		file >> name;
		if(file.eof() || file.fail()) break;
		
		cameras_.insert(std::make_pair(
			name,
			read_camera_(file, dparam)
		));
	}
}


bool tanimoto_camera_array::has(const std::string& name) const {
	return (cameras_.find(name) != cameras_.end());
}


const projection_camera& tanimoto_camera_array::operator[](const std::string& name) const {
	return cameras_.at(name);
}


}

