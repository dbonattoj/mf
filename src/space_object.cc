#include "space_object.h"
#include "geometry/bounding_box.h"
#include "geometry/spherical_coordinates.h"
#include <stdexcept>
#include <mutex>

namespace mf {

space_object::space_object(const pose& ps, space_object& par) :
parent_(&par), pose_(ps) {
	attach_to_parent_();
}


space_object::space_object(const pose& ps) :
parent_(nullptr), pose_(ps) { }


space_object::space_object(const space_object& obj) :
parent_(obj.parent_), pose_(obj.pose_) {
	attach_to_parent_();
}


space_object::~space_object() {
	detach_from_parent_();
	detach_from_children_();
}


void space_object::detach_from_parent_() {
	if(parent_) parent_->children_.erase(this);
}

void space_object::detach_from_children_() {
	for(auto child : children_)
		if(child->parent_ == this) child->parent_ = nullptr;
}


void space_object::attach_to_parent_() {
	if(parent_) parent_->children_.insert(this);
}


bool space_object::has_parent_space_object() const {
	return (parent_ != nullptr);
}


const space_object& space_object::parent_space_object() const {
	if(! has_parent_space_object()) throw std::logic_error("space object has no parent");
	else return *parent_;
}


space_object& space_object::parent_space_object() {
	if(! has_parent_space_object()) throw std::logic_error("space object has no parent");
	else return *parent_;
}

const pose& space_object::relative_pose() const {
	return pose_;
}


pose space_object::absolute_pose() const {
	Eigen::Affine3f trans = pose_.transformation_to_world();
	if(has_parent_space_object())
		trans = parent_space_object().absolute_pose().transformation_to_world() * trans;
	return pose(trans);
}


void space_object::set_relative_pose(const pose& ps) {
	pose_ = ps;
}


void space_object::set_parent(space_object& par, const pose& new_relative_pose) {
	detach_from_parent_();
	parent_ = &par;
	attach_to_parent_();
	pose_ = new_relative_pose;
}


void space_object::set_no_parent(const pose& new_pose) {
	detach_from_parent_();
	pose_ = new_pose;
}


void space_object::make_sibling(const space_object& obj, const pose& new_pose) {
	attach_to_parent_();
	parent_ = obj.parent_;
	pose_ = new_pose;
	detach_from_parent_();
}


void space_object::transform_(const Eigen::Affine3f& t) {
	Eigen::Affine3f pose_transformation = pose_.transformation_to_world();
	pose_transformation = t * pose_transformation;
	pose_ = pose(pose_transformation);
}


bounding_box space_object::box() const {
	// default implementation
	throw std::logic_error("space object has no bounding box");
}


Eigen::Affine3f space_object::transformation_to(const space_object& obj) const {
	Eigen::Affine3f obj_to_world = obj.absolute_pose().transformation_to_world();
	Eigen::Affine3f world_to_this = absolute_pose().transformation_from_world();
	return world_to_this * obj_to_world;
}


Eigen::Affine3f space_object::transformation_from(const space_object& obj) const {
	return obj.transformation_to(*this);
}


void space_object::rotate_x_axis(angle a, const Eigen::Vector3f& c) {
	Eigen::Translation3f t(c + pose_.position);
	transform(t * Eigen::AngleAxisf(a, Eigen::Vector3f::UnitX()) * t.inverse());
}


void space_object::rotate_y_axis(angle a, const Eigen::Vector3f& c) {
	Eigen::Translation3f t(c + pose_.position);
	transform(t * Eigen::AngleAxisf(a, Eigen::Vector3f::UnitY()) * t.inverse());
}


void space_object::rotate_z_axis(angle a, const Eigen::Vector3f& c) {
	Eigen::Translation3f t(c + pose_.position);
	transform(t * Eigen::AngleAxisf(a, Eigen::Vector3f::UnitZ()) * t.inverse());
}



void space_object::look_at(const space_object& obj) {
	Eigen::Vector3f at_obj = transformation_from(obj) * Eigen::Vector3f::Zero();
	Eigen::Vector3f at_depth(0, 0, 1);
	pose_.orientation.setFromTwoVectors(at_depth, at_obj);
	pose_.orientation.normalize();
}


}
