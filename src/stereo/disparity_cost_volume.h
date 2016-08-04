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
