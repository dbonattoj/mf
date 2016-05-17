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
