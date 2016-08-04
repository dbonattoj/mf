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

#include <mf/camera/projection_image_camera.h>
#include <mf/stereo/disparity_cost_volume.h>
#include <mf/io/image_import.h>

using namespace mf;

int main() {
	depth_projection_parameters depth_proj;
	depth_proj.range = depth_projection_parameters::unsigned_normalized_disparity;
	depth_proj.z_near = 27.0;
	depth_proj.z_far = 600.0;
	
	real baseline = 380.135;

	Eigen_mat3 left_intrinsic; left_intrinsic <<
		7315.238, 0.0, 997.555,
		0.0, 7315.238, 980.754,
		0.0, 0.0, 1.0;
	projection_image_camera<int> left_cam(
		pose(Eigen_vec3(-baseline/2.0, 0.0, 0.0), Eigen_quaternion::Identity()),
		left_intrinsic,
		depth_proj,
		make_ndsize(2632, 1988)
	);

	Eigen_mat3 right_intrinsic; right_intrinsic <<
		7315.238, 0.0, 1806.75,
		0.0, 7315.238, 980.754,
		0.0, 0.0, 1.0;
	projection_image_camera<int> right_cam(
		pose(Eigen_vec3(+baseline/2.0, 0.0, 0.0), Eigen_quaternion::Identity()),
		right_intrinsic,
		depth_proj,
		make_ndsize(2632, 1988)
	);
	
	
	auto im_left = image_import("../../imL.png");
	auto im_right = image_import("../../imR.png");
	
	
	auto cost = [](const projection_image_camera<int>& source, const ndptrdiff<2>& source_coord, const projection_image_camera<int>& target, const ndptrdiff<2>& target_coord) {
		return 1.0;
	};

	
	disparity_cost_volume<decltype(cost)> cost_volume(left_cam, right_cam, true, cost);
	cost_volume.cost({100, 100}, 3);
}
