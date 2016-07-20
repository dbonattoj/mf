/*
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
	projection_camera left_cam(
		pose(Eigen_vec2(-baseline/2.0, 0.0, 0.0), Eigen_quaternion::Identity()),
		left_intrinsic,
		depth_proj,
		make_ndsize(2632, 1988)
	);

	Eigen_mat3 right_intrinsic; right_intrinsic <<
		7315.238, 0.0, 1806.75,
		0.0, 7315.238, 980.754,
		0.0, 0.0, 1.0;
	projection_camera right_cam(
		pose(Eigen_vec2(+baseline/2.0, 0.0, 0.0), Eigen_quaternion::Identity()),
		right_intrinsic,
		depth_proj,
		make_ndsize(2632, 1988)
	);
	
	
	auto cost = []() {
		
	};

	
	disparity_cost_volume<decltype(cost)> cost_volume(left_cam, right_cam, true);
}
*/
