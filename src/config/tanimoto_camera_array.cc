#include "tanimoto_camera_array.h"
#include <fstream>
#include <string>
#include "../eigen.h"
#include "../geometry/pose.h"
#include <iostream>

namespace mf {

projection_camera tanimoto_camera_array::read_camera_
(std::istream& str, const depth_projection_parameters& dparam, const ndsize<2>& img_sz) {
	str.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	
	Eigen_mat3 intrinsic = Eigen_mat3::Zero();
	str >> intrinsic(0, 0) >> intrinsic(0, 1) >> intrinsic(0, 2);
	str >> intrinsic(1, 0) >> intrinsic(1, 1) >> intrinsic(1, 2);
	str >> intrinsic(2, 0) >> intrinsic(2, 1) >> intrinsic(2, 2);
	
	float gomi[2]; // ?
	str >> gomi[0] >> gomi[1];

	Eigen_mat3 rotation;
	Eigen_vec3 translation;
	str >> rotation(0, 0) >> rotation(0, 1) >> rotation(0, 2); str >> translation(0);
	str >> rotation(1, 0) >> rotation(1, 1) >> rotation(1, 2); str >> translation(1);
	str >> rotation(2, 0) >> rotation(2, 1) >> rotation(2, 2); str >> translation(2);
	
	Eigen::Affine3f extrinsic_affine;
	extrinsic_affine = Eigen::Translation3f(translation) * Eigen::Affine3f(rotation).inverse();
		
	return projection_camera(extrinsic_affine, intrinsic, dparam, img_sz);
}


tanimoto_camera_array::tanimoto_camera_array
(const std::string& filename, const depth_projection_parameters& dparam, const ndsize<2>& img_sz) {
	std::ifstream file(filename);
	
	for(;;) {
		file.exceptions(std::ios_base::badbit);	

		std::string name;
		file >> name;
		if(file.eof() || file.fail()) break;
		
		cameras_.insert(std::make_pair(
			name,
			read_camera_(file, dparam, img_sz)
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

