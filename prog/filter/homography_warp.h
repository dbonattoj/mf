#ifndef MF_HOMOGRAPHY_WARP_FILTER_H_
#define MF_HOMOGRAPHY_WARP_FILTER_H_

#include <mf/filter/filter.h>
#include <mf/camera/projection_image_camera.h>
#include <mf/color.h>
#include "../support/common.h"
	
class homography_warp_filter : public mf::flow::filter {
public:
	using camera_type = mf::projection_image_camera<depth_type>;
	using color_type = mf::rgba_color;

public:
	input_type<2, color_type> source_image_input;
	input_type<2, depth_type> source_depth_input;
	output_type<2, color_type> destination_image_output;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;
		
	homography_warp_filter(mf::flow::filter_node& nd) :
		mf::flow::filter(nd),
		source_image_input(*this), source_depth_input(*this), destination_image_output(*this) { }

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
