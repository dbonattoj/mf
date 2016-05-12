#include "result_filter_node.h"
#include <mf/image/image.h>
#include <mf/opencv.h>

using namespace mf;


void result_filter_node::setup() {
	output.define_frame_shape(input.frame_shape());
}

void result_filter_node::process(flow::node_job& job) {
	auto in_img = to_image(job.in(input));
	auto out_img = to_image(job.out(output));

	rgb_color bg = rgb_color::black;

	//in_img.write_cv_mat_background(bg);
	//cv::medianBlur(in_img.cv_mat(), in_img.cv_mat(), 5);
	//in_img.read_cv_mat_background(bg);

	in_img.commit_cv_mat();
	out_img.view() = in_img.view();
}
