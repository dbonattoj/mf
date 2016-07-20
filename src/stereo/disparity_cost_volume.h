#ifndef MF_STEREO_DISPARITY_COST_VOLUME_H_
#define MF_STEREO_DISPARITY_COST_VOLUME_H_

#include "../camera/projection_image_camera.h"

namespace mf {

template<typename Cost_function, typename Disparity = int>
class disparity_cost_volume {
public:
	using camera_type = projection_image_camera<Disparity>;
	using disparity_type = Disparity;
	using cost_type = real;
	using coordinates_type = ndptrdiff<2>;

private:
	const camera_type& source_;
	const camera_type& target_;
	Eigen_mat3 fundamental_matrix_;
	bool rectified_;
	Cost_function cost_function_;
	
	//Eigen_line2 target_epipolar_line_(coordinates_type source_coord) const;
	//Eigen_line2 source_epipolar_line_(coordinates_type target_coord) const;
	
	Eigen_vec2 target_matching_coordinates_(Eigen_vec2 source_coord, disparity_type disparity) const;
	Eigen_vec2 source_matching_coordinates_(Eigen_vec2 source_coord, disparity_type disparity) const;
	
public:
	disparity_cost_volume(const camera_type& source, const camera_type& target, bool rectified, const Cost_function&);
	
	cost_type cost(coordinates_type, disparity_type) const;
};

}

#include "disparity_cost_volume.tcc"

#endif
