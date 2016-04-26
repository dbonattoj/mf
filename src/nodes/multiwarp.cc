#include "multiwarp.h"
#include <algorithm>
#include <cmath>
#include "../ndarray/ndarray.h"

#include <iostream>

namespace mf { namespace node {

void multiwarp::setup() {
	out.define_frame_shape(ins[0]->im.frame_shape());
}


void multiwarp::pre_process() {
	time_unit t = current_time();
	
	Eigen_vec3 min_pos = ins.front()->cam.absolute_pose().position;
	Eigen_quaternion min_ori = ins.front()->cam.absolute_pose().orientation;

	Eigen_vec3 max_pos = ins.back()->cam.absolute_pose().position;
	Eigen_quaternion max_ori = ins.back()->cam.absolute_pose().orientation;
	
	real k = (1.0 + std::cos(pi * t / 200.0)*0.7) / 2.0;
	
	Eigen_vec3 pos = min_pos + k*(max_pos - min_pos);
	Eigen_quaternion ori = min_ori.slerp(k, max_ori);
	
	pose ps(pos, ori);
	output_camera_.set_relative_pose(ps);
	
	
	std::size_t n = 3;
	std::vector<std::pair<real, input_prop*>> closest;
	for(const auto& in : ins)
		closest.emplace_back((in->cam.absolute_pose().position - output_camera_.absolute_pose().position).norm(), in.get());
	std::sort(closest.begin(), closest.end(), [](const auto& a, const auto& b) { return (a.first < b.first); });

	for(std::ptrdiff_t i = 0; i < closest.size(); ++i) {
		closest[i].second->im.set_activated(i < n);
		closest[i].second->di.set_activated(i < n);
	}
	
	std::cout << "(";
	for(std::ptrdiff_t i = 0; i < closest.size(); ++i) std::cout << ins[i]->im.is_activated() << "  ";
	std::cout << ")" << std::endl;
}


void multiwarp::warp_(
	const ndarray_view<2, rgba_color> in_im,
	const ndarray_view<2, std::uint8_t> in_di,
	const ndarray_view<2, rgba_color> out_im,
	const camera_type& in_cam,
	const camera_type& out_cam) const
{
	Eigen_projective3 homography = homography_transformation(in_cam, out_cam);
	
	std::fill(out_im.begin(), out_im.end(), rgba_color::null());

	ndarray<2, real> d_buffer(in_im.shape());
	for(real& d : d_buffer) d = 0.0;

	for(std::ptrdiff_t y = 0; y < in_im.shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < in_im.shape()[1]; ++x) {
		auto pix_c = make_ndptrdiff(y, x);
		
		std::uint8_t pix_d = in_di.at(pix_c);
		rgba_color col = in_im.at(pix_c);

		real d = in_cam.to_depth(pix_d);
		auto c = in_cam.to_image(pix_c);

		Eigen_vec3 in(c[0], c[1], d);
		Eigen_vec3 out = (homography * in.homogeneous()).eval().hnormalized();
		
		auto out_pix_c = out_cam.to_pixel(out.head(2));
		
		if(out_cam.image_span().includes(out_pix_c)) {
			real& old_d = d_buffer.at(out_pix_c);
			real new_d = out[2];
			if(new_d > old_d) {
				out_im.at(out_pix_c) = col;
				old_d = new_d;
			}
		}
	}
}


void multiwarp::process() {
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
		
		for(std::ptrdiff_t i = 0; i < images.size(); ++i) {
			rgba_color i_col = images[i][y][x];
			if(i_col.is_null()) continue;
			null = false;
			
			float w = 1.0 / (cams[i]->absolute_pose().position - output_camera_.absolute_pose().position).norm();
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
