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

#ifndef MF_REVERSE_HOMOGRAPHY_FILTER_NODE_H_
#define MF_REVERSE_HOMOGRAPHY_FILTER_NODE_H_

#include <mf/filter/filter.h>
#include <mf/camera/projection_image_camera.h>
#include <mf/color.h>
#include <mf/masked_elem.h>
#include "../support/common.h"
	
class reverse_homography_warp_filter : public mf::flow::filter {
	using camera_type = mf::projection_image_camera<depth_type>;
	using color_type = mf::rgb_color;
	using masked_depth_type = mf::masked_elem<depth_type>;
	using masked_image_type = mf::masked_elem<color_type>;

public:
	input_type<2, color_type> source_image_input;
	input_type<2, masked_depth_type> destination_depth_input;
	output_type<2, masked_image_type> destination_image_output;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;
		
	reverse_homography_warp_filter(mf::flow::filter_node& nd) :
		mf::flow::filter(nd),
		source_image_input(*this), destination_depth_input(*this), destination_image_output(*this) { }

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
