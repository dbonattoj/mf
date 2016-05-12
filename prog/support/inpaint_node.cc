#include "inpaint_node.h"
#include <mf/image/image.h>
#include <mf/opencv.h>

using namespace mf;


void inpaint_node::setup() {
	output.define_frame_shape(input.frame_shape());
}

void inpaint_node::process(flow::node_job& job) {
	auto in_img = to_image(job.in(input));
	auto out_img = to_image(job.out(output));

	cv::Mat_<uchar> mask = in_img.cv_mask_mat();
	mask = true;
	
	//cv::inpaint(in_img.cv_mat(), mask, out_img.cv_mat(), 1, cv::INPAINT_TELEA);

	in_img.cv_mat().copyTo(out_img.cv_mat(), mask);
	mask.copyTo(out_img.cv_mask_mat());

	out_img.commit_cv_mat();
}
