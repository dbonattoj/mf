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

#include "blend_closest.h"
#include <algorithm>
#include <cmath>
#include <mf/ndarray/ndarray.h>

using namespace mf;

namespace {
	real camera_distance(const blend_closest_filter::camera_type& a, const blend_closest_filter::camera_type& b) {
		return (a.absolute_pose().position - b.absolute_pose().position).norm();
	}
}

void blend_closest_filter::setup() {
	output.define_frame_shape(visuals_[0]->image_input.frame_shape());
}


void blend_closest_filter::pre_process(flow::node_job& job) {	
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



void blend_closest_filter::process(flow::node_job& job) {	
	auto out = job.out(output);
	
	std::vector<ndarray_view<2, masked_elem<rgb_color>>> act_ins;
	for(active_input_visual act_vis : active_visuals_)
		act_ins.push_back(job.in(act_vis.visual->image_input));
	
	for(std::ptrdiff_t y = 0; y < out.shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < out.shape()[1]; ++x) {
		real sr = 0, sg  = 0, sb = 0, total = 0;
		bool null = true;
		
		for(std::ptrdiff_t i = 0; i < act_ins.size(); ++i) {
			const auto& im = act_ins[i];
			const auto& act_vis = active_visuals_[i];
			
			auto i_col = im[y][x];
			if(i_col.is_null()) continue;
			null = false;
			
			real w = 1.0 / act_vis.camera_distance;
			sr += w * i_col.elem.r;
			sg += w * i_col.elem.g;
			sb += w * i_col.elem.b;
			total += w;
		}
		
		if(null) {
			out[y][x] = masked_elem<rgb_color>::null();
		} else {
			sr /= total;
			sg /= total;
			sb /= total;
			out[y][x] = rgb_color(sr, sg, sb);
		}
	}
}
