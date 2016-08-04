/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

