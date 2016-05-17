#include "downscale.h"
#include <mf/image/image.h>
#include <mf/opencv.h>

using namespace mf;


void downscale_filter::setup() {
	output.define_frame_shape(output_size_);
}

void downscale_filter::process(mf::flow::node_job& job) {
	auto in_img = to_image(job.in(input));
	auto out_img = to_image(job.out(output));

	cv::resize(in_img.cv_mat(), out_img.cv_mat(), cv::Size(output_size_[0], output_size_[1]));
	out_img.commit_cv_mat();
}
