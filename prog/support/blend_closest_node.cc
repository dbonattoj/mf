#include "blend_closest_node.h"
#include <algorithm>
#include <cmath>
#include <mf/ndarray/ndarray.h>

using namespace mf;

namespace {
	real camera_distance(const blend_closest_node::camera_type& a, const blend_closest_node::camera_type& b) {
		return (a.absolute_pose().position - b.absolute_pose().position).norm();
	}
}

void blend_closest_node::setup() {
	output.define_frame_shape(visuals_[0]->image_input.frame_shape());
}


void blend_closest_node::pre_process(flow::node_job& job) {	
	time_unit t = job.time();
	auto out_cam = job.param(output_camera);
		
	// activate only number_of_active_inputs_ closest input visuals
	active_visuals_.clear();
	for(const auto& visual : visuals_) {
		active_visuals_.emplace_back(
			visual.get(),
			camera_distance(out_cam, job.param(visual->camera))
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
