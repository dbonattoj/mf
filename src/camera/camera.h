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

#ifndef MF_CAMERA_H_
#define MF_CAMERA_H_

#include "../eigen.h"
#include <array>
#include <utility>
#include "../space_object.h"
#include "../geometry/spherical_coordinates.h"
#include "../eigen.h"

namespace mf {

/// Camera which defines mapping from 3D spatial coordinates to 2D image coordinates.
/** Inherited space object pose defines extrinsic camera parameters. Camera points in -Z or +Z direction in its
 ** coordinate system. 3D points taken as function arguments are always in coordinate system of parent space object. */
class camera : public space_object {
protected:
	camera() = default;
	explicit camera(const pose&);

public:
	camera(const camera&) = default;
	camera& operator=(const camera&) = default;

	using image_coordinates_type = Eigen_vec2; ///< 2D image coordinates, range and scale defined by subclass.

	virtual ~camera() = default;

	/// Extrinsic parameters of camera.
	/** Transformation from parent space object coordinate system to coordinate system of the camera. */
	Eigen_affine3 view_transformation() const;
		
	/// Direction vector of ray -Z pointing straight out camera.
	/** Relative to parent space object coordinate system. */
	Eigen_vec3 center_ray_direction() const {
		return ray_direction(Eigen_vec3(0, 0, -1));
	}

	/// Direction vector of ray pointing to point with spherical coordinates \a sp.
	Eigen_vec3 ray_direction(const spherical_coordinates& sp) const {
		return ray_direction(point(sp));
	}

	/// Direction vector of ray pointing to point \a p.
	Eigen_vec3 ray_direction(const Eigen_vec3& p) const;

	/// Squared distance of 3D point \a p to camera center.
	float distance_sq(const Eigen_vec3& p) const;

	/// Distance of 3D point \a p to camera center.
	float distance(const Eigen_vec3& p) const;	

	/// Convert 3D point cartesian coordinates \a p to spherical.
	spherical_coordinates to_spherical(const Eigen_vec3& p) const;
	
	/// Convert 3D point spherical coordinates \a sp to cartesian.
	Eigen_vec3 point(const spherical_coordinates& sp) const;
	
	/// Project point \a p to image coordinates.
	/** Implemented by subclass. */
	virtual image_coordinates_type project(const Eigen_vec3& p) const = 0;
	
	/// Project point with spherical coordinates \a sp to image coordinates.
	/** Subclass may implemented more efficient version. */
	virtual image_coordinates_type project(const spherical_coordinates& sp) const {
		return this->project(point(sp));
	}

	/// Direction vector of ray pointing to point corresponding to image coordinates \a c.
	/** Implemented by subclass. */
	virtual Eigen_vec3 ray_direction(const image_coordinates_type& c) const = 0;
};


Eigen_mat3 essential_matrix(const camera& from, const camera& to);


}

#endif
