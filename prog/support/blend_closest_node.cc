#include "multiwarp.h"
#include <algorithm>
#include <cmath>
#include "../../src/ndarray/ndarray.h"

namespace mf { namespace node {

void blend_closest_node::setup() {
	out.define_frame_shape(ins[0]->im.frame_shape());
}


void blend_closest_node::pre_process() {
	time_unit t = current_time();
	
	// vary output camera pose
	// TODO node parameter system
	Eigen_vec3 min_pos = ins.front()->cam.absolute_pose().position;
	Eigen_quaternion min_ori = ins.front()->cam.absolute_pose().orientation;

	Eigen_vec3 max_pos = ins.back()->cam.absolute_pose().position;
	Eigen_quaternion max_ori = ins.back()->cam.absolute_pose().orientation;
	
	real k = (1.0 + std::cos(pi * t / 200.0)*0.7) / 2.0;
	
	Eigen_vec3 pos = min_pos + k*(max_pos - min_pos);
	Eigen_quaternion ori = min_ori.slerp(k, max_ori);
	
	pose ps(pos, ori);
	output_camera_.set_relative_pose(ps);
	
	// activate only number_of_active_inputs_ closest input visuals
	active_visuals_.clear();
	for(const auto& in : ins) {
		active_visuals_.emplace_back(
			in.get(),
			(in->cam.absolute_pose().position - output_camera_.absolute_pose().position).norm()
		);
	}
	std::sort(
		active_visuals_.begin(),
		active_visuals_.end(),
		[](const auto& a, const auto& b) { return (a.first < b.first); }
	);
	for(std::ptrdiff_t i = 0; i < active_visuals_.size(); ++i) {
		active_visuals_[i].visual->image_in.set_activated(i < number_of_active_inputs_);
		active_visuals_[i].visual->depth_in.set_activated(i < number_of_active_inputs_);
	}
	active_visuals_.erase(active_visuals_.begin() + number_of_active_inputs_, active_visuals_.end());
}



void blend_closest_node::process() {
	std::cout << "frame " << current_time() << std::endl;
	
	std::vector<ndarray<2, rgba_color>> images;
	std::vector<camera_type*> cams;
	for(const auto& in : ins) {
		if(in->im.is_activated()) {
			cams.emplace_back(&in->cam);
			images.emplace_back(in->im.frame_shape());
			warp_(in->im.view(), in->di.view(), images.back().view(), in->cam, output_camera_);
		}
	}
	
	for(std::ptrdiff_t y = 0; y < out.view().shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < out.view().shape()[1]; ++x) {
		real sr = 0, sg  = 0, sb = 0, total = 0;
		bool null = true;
		
		for(active_input_visual act_vis : active_visuals_) {
			rgba_color i_col = act_vis.visual->input_in[y][x];
			if(i_col.is_null()) continue;
			null = false;
			
			float w = 1.0 / act_vis.distance;
			sr += w * i_col.r;
			sg += w * i_col.g;
			sb += w * i_col.b;
			total += w;
		}
		
		if(null) {
			out.view()[y][x] = rgba_color::null();
		} else {
			sr /= total;
			sg /= total;
			sb /= total;
			out.view()[y][x] = rgba_color(sr, sg, sb);
		}
	}
}


}}
