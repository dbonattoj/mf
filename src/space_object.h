#ifndef MF_SPACE_OBJECT_H_
#define MF_SPACE_OBJECT_H_

#include "geometry/pose.h"
#include "geometry/angle.h"
#include "eigen.h"
#include <set>
#include <string>
#include <mutex>

namespace mf {

struct bounding_box;

/// Base class for object that has a pose in 3D space relative to a coordinate system.
/** Can have a parent space object, meaning its pose will be defined relative to that parent.
 ** No parent means pose is absolute. Parent can be reassigned. Keeps references to its children,
 ** Guarantees that pointers to parent or children get properly removed when either child or parent gets deleted.*/
class space_object {
private:
	space_object* parent_;
	std::set<space_object*> children_;
	pose pose_;
	
	void transform_(const Eigen_affine3&);
	
	void detach_from_parent_();
	void detach_from_children_();
	void attach_to_parent_();

protected:
	virtual void do_update_pose() { }
		
public:
	explicit space_object(const pose& = pose());
	space_object(const pose&, space_object& par);

	space_object(const space_object&);
	space_object& operator=(const space_object&);
	virtual ~space_object();

	bool has_parent_space_object() const;
	const space_object& parent_space_object() const;
	space_object& parent_space_object();
	
	const pose& relative_pose() const;
	pose absolute_pose() const;
	
	void set_relative_pose(const pose&);
	void set_no_relative_pose() { set_relative_pose(pose()); }
	
	Eigen_affine3 transformation_from(const space_object&) const;
	Eigen_affine3 transformation_to(const space_object&) const;
	
	void set_parent(space_object&, const pose& new_relative_pose = pose());
	void set_no_parent(const pose& new_pose = pose());
	
	void make_sibling(const space_object&, const pose& new_pose = pose());
				
	template<typename Transformation>
	void transform(const Transformation& t) {
		transform_(Eigen_affine3(t));
	}
	
	template<typename Transformation>
	void transform(const Transformation& rt, const space_object& relative_to) {
		Eigen_affine3 btw = relative_to.absolute_pose().transformation_to_world();
		Eigen_affine3 t = btw * rt * btw.inverse();
		transform_(t);
	}
	
	void move(const Eigen_vec3& t) { transform(Eigen_translation3(t)); }
	void move(float x, float y, float z) { move(Eigen_vec3(x, y, z)); }
	void move_x(float x) { move(x, 0, 0); }
	void move_y(float y) { move(0, y, 0); }
	void move_z(float z) { move(0, 0, z); }
	
	void rotate_x_axis(angle a, const Eigen_vec3& c = Eigen_vec3::Zero());
	void rotate_y_axis(angle a, const Eigen_vec3& c = Eigen_vec3::Zero());
	void rotate_z_axis(angle a, const Eigen_vec3& c = Eigen_vec3::Zero());
	
	void look_at(const space_object&);
	
	virtual bounding_box box() const;
};


inline Eigen_affine3 pose_transformation(const space_object& from, const space_object& to) {
	return from.transformation_to(to);
}



}

#endif
