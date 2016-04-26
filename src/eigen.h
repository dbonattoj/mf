#ifndef MF_EIGEN_H_
#define MF_EIGEN_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-register"
// Eigen uses C++14 deprecated 'register' keyword

#include <Eigen/Eigen>
#include <Eigen/Geometry>

#pragma GCC diagnostic pop

#include "common.h"
#include "elem.h"

namespace mf {
	
using Eigen_scalar = real;

template<std::size_t Rows, std::size_t Cols>
using Eigen_mat = Eigen::Matrix<Eigen_scalar, Rows, Cols>;
	
using Eigen_vec2 = Eigen_mat<2, 1>;
using Eigen_vec3 = Eigen_mat<3, 1>;
using Eigen_vec4 = Eigen_mat<4, 1>;
	
using Eigen_mat2 = Eigen_mat<2, 2>;
using Eigen_mat3 = Eigen_mat<3, 3>;
using Eigen_mat4 = Eigen_mat<4, 4>;

using Eigen_translation2 = Eigen::Translation<Eigen_scalar, 2>;
using Eigen_translation3 = Eigen::Translation<Eigen_scalar, 3>;
using Eigen_angleaxis = Eigen::AngleAxis<Eigen_scalar>;

using Eigen_affine2 = Eigen::Transform<Eigen_scalar, 2, Eigen::Affine>;
using Eigen_affine3 = Eigen::Transform<Eigen_scalar, 3, Eigen::Affine>;

using Eigen_projective2 = Eigen::Transform<Eigen_scalar, 2, Eigen::Projective>;
using Eigen_projective3 = Eigen::Transform<Eigen_scalar, 3, Eigen::Projective>;

using Eigen_hyperplane2 = Eigen::Hyperplane<Eigen_scalar, 2>;
using Eigen_hyperplane3 = Eigen::Hyperplane<Eigen_scalar, 3>;

using Eigen_quaternion = Eigen::Quaternion<Eigen_scalar>;


template<typename Scalar, std::size_t Rows, std::size_t Columns>
struct elem_traits<Eigen::Matrix<Scalar, Rows, Columns>> :
	elem_traits_base<
		Eigen::Matrix<Scalar, Rows, Columns>,
		Scalar,
		Rows * Columns,
		false
	> { };


}

#endif
