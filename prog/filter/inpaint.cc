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

#include "inpaint.h"
#include <mf/image/image.h>
#include <mf/opencv.h>

using namespace mf;


void inpaint_filter::setup() {
	output.define_frame_shape(input.frame_shape());
}

void inpaint_filter::process(mf::flow::node_job& job) {
	auto in_img = to_image(job.in(input));
	auto out_img = to_image(job.out(output));

	cv::Mat_<uchar> mask = in_img.cv_mask_mat();
	mask = true;
	
	//cv::inpaint(in_img.cv_mat(), mask, out_img.cv_mat(), 1, cv::INPAINT_TELEA);

	in_img.cv_mat().copyTo(out_img.cv_mat(), mask);
	mask.copyTo(out_img.cv_mask_mat());

	out_img.commit_cv_mat();
}
