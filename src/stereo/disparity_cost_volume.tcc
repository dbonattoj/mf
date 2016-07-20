namespace mf {

/*
template<typename Cost_function, typename Disparity>
Eigen_line2 disparity_cost_volume<Cost_function, Disparity>::target_epipolar_line_(coordinates_type source_coord) const {
	
}


template<typename Cost_function, typename Disparity>
Eigen_line2 disparity_cost_volume<Cost_function, Disparity>::source_epipolar_line_(coordinates_type source_coord) const {
	
}
*/

template<typename Cost_function, typename Disparity>
Eigen_vec2 disparity_cost_volume<Cost_function, Disparity>::target_matching_coordinates_
(Eigen_vec2 source_coord, disparity_type disparity) const {
	return Eigen_vec2(source_coord[0] + disparity, source_coord[1]);
}


template<typename Cost_function, typename Disparity>
Eigen_vec2 disparity_cost_volume<Cost_function, Disparity>::source_matching_coordinates_
(Eigen_vec2 source_coord, disparity_type disparity) const {
	
}



template<typename Cost_function, typename Disparity>
disparity_cost_volume<Cost_function, Disparity>::disparity_cost_volume
(const camera_type& source, const camera_type& target, bool rectified_, const Cost_function& cost_function) :
	source_(source),
	target_(target),
	fundamental_matrix_(fundamental_matrix(source_, target_)),
	rectified_(rectified_),
	cost_function_(cost_function)
{
	Expects(source_.image_size() == target_.image_size());
}


template<typename Cost_function, typename Disparity>
auto disparity_cost_volume<Cost_function, Disparity>::cost
(coordinates_type source_pix_coord, disparity_type disparity) const -> cost_type {
	Eigen_vec2 source_coord = source_.to_image(source_pix_coord);
	Eigen_vec2 target_coord = target_matching_coordinates_(source_coord, disparity);
	coordinates_type target_pix_coord = target_.to_pixel(target_coord);
	return cost_function_(source_, source_pix_coord, target_, target_pix_coord);
}


}

