#include "blend_closest_node.h"
#include <algorithm>
#include <cmath>
#include "../../src/ndarray/ndarray.h"

namespace mf { namespace node {

void blend_closest_node::setup() {
	output.define_frame_shape(visuals_[0]->image_input.frame_shape());
}


void blend_closest_node::pre_process(time_unit t) {	
	// vary output camera pose
	// TODO node parameter system
	Eigen_vec3 min_pos = visuals_.front()->camera.absolute_pose().position;
	Eigen_quaternion min_ori = visuals_.front()->camera.absolute_pose().orientation;

	Eigen_vec3 max_pos = visuals_.back()->camera.absolute_pose().position;
	Eigen_quaternion max_ori = visuals_.back()->camera.absolute_pose().orientation;
	
	real k = (1.0 + std::cos(pi * t / 200.0)*0.7) / 2.0;
	
	Eigen_vec3 pos = min_pos + k*(max_pos - min_pos);
	Eigen_quaternion ori = min_ori.slerp(k, max_ori);
	
	pose ps(pos, ori);
	output_camera_.set_relative_pose(ps);
	
	// activate only number_of_active_inputs_ closest input visuals
	active_visuals_.clear();
	for(const auto& visual : visuals_) {
		active_visuals_.emplace_back(
			visual.get(),
			(visual->camera.absolute_pose().position - output_camera_.absolute_pose().position).norm()
		);
	}
	std::sort(
		active_visuals_.begin(),
		active_visuals_.end(),
		[](const auto& a, const auto& b) { return (a.camera_distance < b.camera_distance); }
	);
	for(std::ptrdiff_t i = 0; i < active_visuals_.size(); ++i) {
		active_visuals_[i].visual->image_input.set_activated(i < number_of_active_inputs_);
	}
	active_visuals_.erase(active_visuals_.begin() + number_of_active_inputs_, active_visuals_.end());
}



void blend_closest_node::process(flow::node_job& job) {
	std::cout << "frame " << job.time() << std::endl;
	
	auto out = job.out(output);
	
	for(std::ptrdiff_t y = 0; y < out.shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < out.shape()[1]; ++x) {
		real sr = 0, sg  = 0, sb = 0, total = 0;
		bool null = true;
		
		for(active_input_visual act_vis : active_visuals_) {
			auto im = job.in(act_vis.visual->image_input);
			
			rgba_color i_col = im[y][x];
			if(i_col.is_null()) continue;
			null = false;
			
			float w = 1.0 / act_vis.camera_distance;
			sr += w * i_col.r;
			sg += w * i_col.g;
			sb += w * i_col.b;
			total += w;
		}
		
		if(null) {
			out[y][x] = rgba_color::null();
		} else {
			sr /= total;
			sg /= total;
			sb /= total;
			out[y][x] = rgba_color(sr, sg, sb);
		}
	}
}


}}
