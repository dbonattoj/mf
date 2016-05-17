#ifndef MF_HOMOGRAPHY_DEPTH_WARP_FILTER_H_
#define MF_HOMOGRAPHY_DEPTH_WARP_FILTER_H_

#include <mf/filter/filter.h>
#include <mf/camera/projection_image_camera.h>
#include <mf/color.h>
#include <mf/masked_elem.h>
#include "../support/common.h"

class homography_depth_warp_filter : public mf::flow::filter {
public:
	using camera_type = mf::projection_image_camera<depth_type>;
	using masked_depth_type = mf::masked_elem<depth_type>;

public:
	input_type<2, masked_depth_type> source_depth_input;
	output_type<2, masked_depth_type> destination_depth_output;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;
		
	homography_depth_warp_filter(mf::flow::filter_node& nd) :
		mf::flow::filter(nd),
		source_depth_input(*this), destination_depth_output(*this) { }

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
