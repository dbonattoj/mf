#ifndef MF_EIGEN_H_
#define MF_EIGEN_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-register"

#include <Eigen/Eigen>
#include <Eigen/Geometry>

#pragma GCC diagnostic pop

#include "elem.h"

namespace mf {
	
using Eigen_scalar = float;
	
using Eigen_vec2 = Eigen::Matrix<Eigen_scalar, 2, 1>;
using Eigen_vec3 = Eigen::Matrix<Eigen_scalar, 3, 1>;
using Eigen_vec4 = Eigen::Matrix<Eigen_scalar, 4, 1>;
	
using Eigen_mat2 = Eigen::Matrix<Eigen_scalar, 2, 2>;
using Eigen_mat3 = Eigen::Matrix<Eigen_scalar, 3, 3>;
using Eigen_mat4 = Eigen::Matrix<Eigen_scalar, 4, 4>;
// TODO complete&use	


template<typename Scalar, std::size_t Rows, std::size_t Columns>
struct elem_traits<Eigen::Matrix<Scalar, Rows, Columns>> :
	elem_traits_base<
		Eigen::Matrix<Scalar, Rows, Columns>,
		Scalar,
		Rows * Columns
	> { };


}

#endif
